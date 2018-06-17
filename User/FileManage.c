/****************************************
**  FileManage.c
**  Auther: ZhouJi
**  Date:   2018-6-6
****************************************/

/*! Includes ---------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "FileManage.h"
/*! Marco -----------------------------*/
#define READ_BACK_TEST		1

/*! Type ------------------------------*/
struct File_Pointer
{
    uint32_t firstFile_ptr;
    uint32_t lastFile_ptr;
    uint32_t number_of_files;
};

// file pointer, pointing the first file address and the last file address
static struct File_Pointer fileRecord = {0xFFFFFFFF, 0xFFFFFFFF, 0x00};
static uint32_t fileRecordAddr;
static uint32_t preFileAddress;
// file operator pointer
static MY_FILE_t readFileStu = {0xFFFFFFFF,};
static MY_FILE_t writeFileStu = {0xFFFFFFFF,};

//test use buffer
#if READ_BACK_TEST
static uint8_t testBuf[64];
#endif
/*! Static Function ---------------------------------------*/
void FileManage_updateRecord(uint32_t addr);
void Filemanage_saveRecord(void);
/*! Function ----------------------------------------------*/

uint16_t CRC16_Check(uint8_t *data, uint32_t len)
{
    uint16_t ret = 0;
    int i;

    while(len--)
    {
        ret ^= ((uint16_t)(*data++) << 8);
        for(i=0;i<8;i++)
        {
            if(ret & 0x8000)
            {
                ret = (ret << 1) ^ 0x1021;
            }
            else
            {
                ret = ret << 1;
            }
        }
    }
    return ret;
}

// static function
static int _Flash_readData(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t size, ret = 0;

    if(addr > MYFLASH_MAX_ADDR) return ret;
    if(len > MYFLASH_MAX_ADDR - MYFLASH_MIN_ADDR - RECORD_SECTOR_SIZE) return ret;
    
    // read file
    if(addr + len > MYFLASH_MAX_ADDR)
    {
        size = MYFLASH_MAX_ADDR - addr;
        ret = Flash_readData(addr, &data[0], size);
        ret += Flash_readData(MYFLASH_MIN_ADDR + RECORD_SECTOR_SIZE, &data[size], len - size);
    }
    else
    {
        ret = Flash_readData(addr, &data[0], len);
    }

    return ret;
}

// FileManage Initialize
void FileManage_Initialize(void)
{
    uint32_t readAddr = RECORD_SECTOR_ADDR;
    struct File_Pointer fpoint;
	file_header_t tFileHead = {0x00, 0x00, 0x00};
	uint8_t tmp[PAGE_SIZE];
	uint32_t readSize, checkSize;

    printlog("\r\nFile manage initialize!");
	Flash_Initialize();

    do
    {
        if(0 > Flash_readData(readAddr, (uint8_t *)&fpoint, sizeof(struct File_Pointer)))
        {
            printerr("\r\nRead data fail!");
            return;
        }

        if(fpoint.firstFile_ptr < RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE || fpoint.firstFile_ptr > MYFLASH_MAX_ADDR)
        {
            printmsg("\r\nFile manage message address is 0x%X", fptr - 1);
            break;
        }
        else
        {
            fileRecord.firstFile_ptr = fpoint.firstFile_ptr;
            fileRecord.lastFile_ptr = fpoint.lastFile_ptr;
            fileRecord.number_of_files = fpoint.number_of_files;
        }
        readAddr += sizeof(struct File_Pointer);
    }while(readAddr + sizeof(struct File_Pointer) < RECORD_SECTOR_SIZE);

    if(readAddr + sizeof(struct File_Pointer) - RECORD_SECTOR_ADDR > RECORD_SECTOR_SIZE)
    {
        fileRecordAddr = RECORD_SECTOR_ADDR;
    }
    else
    {
        fileRecordAddr = readAddr;
    }
	// check the unusual power down
	if(fileRecord.firstFile_ptr >= RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE && fileRecord.firstFile_ptr < MYFLASH_MAX_ADDR)
	{
		_Flash_readData(fileRecord.lastFile_ptr, (uint8_t *)&tFileHead, sizeof(file_header_t));
		if(tFileHead.tag == 0xFFFF)		// unusual power down
		{
			readAddr = fileRecord.lastFile_ptr + sizeof(file_header_t);
			if(readAddr >= MYFLASH_MAX_ADDR)
			{
				readAddr = readAddr - MYFLASH_MAX_ADDR + RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE;
			}
			while((readAddr & (MYFLASH_SECTOR_SIZE-1)) != (fileRecord.firstFile_ptr & (MYFLASH_SECTOR_SIZE-1)))
			{
				readSize = PAGE_SIZE - (readAddr & (PAGE_SIZE - 1));
				_Flash_readData(readAddr, tmp, readSize);
				for(checkSize = 0; checkSize < readSize; checkSize++)
				{
					if(tmp[checkSize] == 0xFF) break;
				}
				
				readAddr += checkSize;
				if(checkSize == readSize)
				{
					if(readAddr >= MYFLASH_MAX_ADDR)
					{
						readAddr = readAddr - MYFLASH_MAX_ADDR + RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE;
					}
				}
				else	// check complete
				{
					if(readAddr < fileRecord.lastFile_ptr)
					{
						tFileHead.fileLen = MYFLASH_MAX_ADDR - fileRecord.lastFile_ptr + readAddr - RECORD_SECTOR_ADDR - RECORD_SECTOR_SIZE;
					}
					else
					{
						tFileHead.fileLen = readAddr - fileRecord.lastFile_ptr;
					}
					break;
				}
			}
			// 
			tFileHead.tag = FILE_HEAD_TAG;
			tFileHead.fileName = 0x00;
			Flash_writeData(fileRecord.lastFile_ptr, (uint8_t *)&tFileHead, sizeof(file_header_t));
		}
	}
	
    printmsg("\r\nFile manage initialize success!");
    return;
}

// file manage -> open a file
MY_FILE_t * FileManage_open(char *name, uint8_t mode)
{
    int len;
    uint32_t fileIndex, fileAddr, address;
    uint16_t nameCheck;
    file_header_t fileptr;

    len = strlen(name);
    if(len <= 0)
    {
        return NULL;
    }

    nameCheck = CRC16_Check((uint8_t *)name, strlen(name));
    if(mode == FILE_OPEN_MODE_WRITE)
    {
        if(writeFileStu.fileAddr == 0xFFFFFFFF)
        {
			if(fileRecord.lastFile_ptr < RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE || fileRecord.lastFile_ptr >= MYFLASH_MAX_ADDR)
			{
				address = RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE;
				writeFileStu.fileAddr = address + sizeof(file_header_t);
				fileRecord.firstFile_ptr = address;
				Flash_eraseSector(address);
			}
			else
			{
				_Flash_readData(fileRecord.lastFile_ptr, (uint8_t *)&fileptr, sizeof(file_header_t));
				address = fileRecord.lastFile_ptr + fileptr.fileLen;
				writeFileStu.fileAddr = address + sizeof(file_header_t);
			}
			
			fileRecord.lastFile_ptr = address;
			
            if(writeFileStu.fileAddr >= MYFLASH_MAX_ADDR)
            {
                writeFileStu.fileAddr = writeFileStu.fileAddr - MYFLASH_MAX_ADDR + MYFLASH_MIN_ADDR;
                FileManage_updateRecord(writeFileStu.fileAddr);
            }
            else if((writeFileStu.fileAddr & ~(MYFLASH_MIN_ADDR - 1)) > (address & ~(MYFLASH_MIN_ADDR - 1)))
            {
                FileManage_updateRecord(writeFileStu.fileAddr);
            }
			else
			{
				fileRecord.number_of_files++;
				Filemanage_saveRecord();
			}
            writeFileStu.fileName = nameCheck;
			writeFileStu.fileSize = 0;
            writeFileStu.readptr = 0;
            return &writeFileStu;
        }
    }
    else if(mode == FILE_OPEN_MODE_READ)
    {
        fileIndex = atoi(name);

        if(fileIndex > 0 && fileIndex <= fileRecord.number_of_files && readFileStu.fileAddr == 0xFFFFFFFF)
        {
            fileAddr = fileRecord.firstFile_ptr;
            do
            {
                _Flash_readData(fileAddr, (uint8_t *)&fileptr, sizeof(file_header_t));
                fileAddr += fileptr.fileLen;
                if(fileAddr >= MYFLASH_MAX_ADDR)
                {
                    fileAddr = fileAddr - MYFLASH_MAX_ADDR + MYFLASH_MIN_ADDR;
                }
				if(fileptr.tag != FILE_HEAD_TAG) return NULL;
            }while(--fileIndex);

            readFileStu.fileAddr = fileAddr + sizeof(file_header_t);
            readFileStu.fileSize = fileptr.fileLen - sizeof(file_header_t);
            readFileStu.fileName = fileptr.fileName;
            readFileStu.readptr = 0;

            return &readFileStu;
        }
    }
    
    return NULL;
}

// close file
void FileManage_close(MY_FILE_t *file)
{
    file_header_t fileH;
    uint32_t addr;
    uint32_t size;

    if(file == &writeFileStu)
    {
        fileH.tag = FILE_HEAD_TAG;
        fileH.fileName = writeFileStu.fileName;
        fileH.fileLen = writeFileStu.fileSize + sizeof(file_header_t);

        addr = writeFileStu.fileAddr - sizeof(file_header_t);
        if(addr < RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE)
        {
            addr += MYFLASH_MAX_ADDR - (RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE);
        }

        // write data
        if((addr & ~(FLASH_SECTOR_ERASE - 1)) != (writeFileStu.fileAddr & ~(FLASH_SECTOR_ERASE - 1)))
        {
            size = MYFLASH_MIN_ADDR - (addr & (MYFLASH_MIN_ADDR - 1));
            Flash_writeData(addr, (uint8_t *)&fileH, size);
            Flash_writeData(addr + size, (uint8_t *)&fileH + size, sizeof(file_header_t) - size);
        }
        else
        {
            Flash_writeData(addr, (uint8_t *)&fileH, sizeof(file_header_t));
        }
        writeFileStu.fileAddr = 0xFFFFFFFF;

        //fileRecord.lastFile_ptr = addr;
        //fileRecord.number_of_files++;
        //Filemanage_saveRecord();

        return;
    }
    else if(file == &readFileStu)
    {
        readFileStu.fileAddr = 0xFFFFFFFF;
        return;
    }
    return;
}

// filemanage -> numbers of file
int FileManage_files(void)
{
    return fileRecord.number_of_files;
}

int FileManage_seek(MY_FILE_t *file, uint32_t seek)
{
    if(file == &readFileStu)
    {
        if(seek < readFileStu.fileSize)
        {
            readFileStu.readptr = seek;
            return seek;
        }
    }
    else if(file == &writeFileStu)
    {
        if(seek < writeFileStu.fileSize)
        {
            writeFileStu.readptr = seek;
            return seek;
        }
    }
    return -1;
}
// file manage -> write file data message
int FileManage_write(MY_FILE_t *file, uint8_t *data, uint32_t len)
{
    int size, ret = 0;
    uint32_t address;
	
    if(file != &writeFileStu) return -1;

    do
    {
		address = writeFileStu.fileAddr + writeFileStu.fileSize;
		if(address >= MYFLASH_MAX_ADDR)
        {
			address = address - MYFLASH_MAX_ADDR + RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE;
		}
        if((address & (MYFLASH_MIN_ADDR - 1)) == 0)
        {
            // new sector
            FileManage_updateRecord(address);
        }

        // start writing data
        size = address & (MYFLASH_PAGE_SIZE-1);
        if(size + len > MYFLASH_PAGE_SIZE)
        {
            size = MYFLASH_PAGE_SIZE - size;
            Flash_writeData(address, data, size);
			#if READ_BACK_TEST
			Flash_readData(address, (uint8_t *)&testBuf[0], size);
			#endif
        }
        else
        {
			size = len;
            Flash_writeData(address, data, size);
			#if READ_BACK_TEST
			Flash_readData(address, (uint8_t *)&testBuf[0], size);
			#endif
        }
		// 
		data += size;
        len -= size;
        ret += size;
		writeFileStu.fileSize += size;
		
    }while(len > 0);

    return ret;
}

// file manage -> read file
int FileManage_read(MY_FILE_t *file, uint8_t *data, uint32_t len)
{
    int ret = 0;

    // no file
    if(file == NULL) return 0;

    // read file data
    if(file == &readFileStu)
    {
        if(len > file->fileSize - file->readptr)
        {
            len = file->fileSize - file->readptr;
        }
        ret = _Flash_readData(file->fileAddr + file->readptr, data, len);
    }
    else if(file == &writeFileStu)
    {
        if(len > file->fileSize - file->readptr)
        {
            len = file->fileSize - file->readptr;
        }
        ret = _Flash_readData(file->fileAddr + file->readptr, data, len);
    }

    return ret;
}

// write file
static void FileManage_updateRecord(uint32_t addr)
{
    file_header_t filehead;

    addr &= ~(MYFLASH_MIN_ADDR - 1);
	
	if(fileRecord.firstFile_ptr > RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE && fileRecord.firstFile_ptr < MYFLASH_MAX_ADDR)
	{
		while(addr == (fileRecord.firstFile_ptr & ~(MYFLASH_MIN_ADDR - 1)))
		{
			_Flash_readData(fileRecord.firstFile_ptr, (uint8_t *)&filehead, sizeof(file_header_t));
			fileRecord.number_of_files--;
			fileRecord.firstFile_ptr += filehead.fileLen;
			if(fileRecord.firstFile_ptr >= MYFLASH_MAX_ADDR)
			{
				fileRecord.firstFile_ptr -= MYFLASH_MAX_ADDR - MYFLASH_MIN_ADDR - RECORD_SECTOR_SIZE;
			}
		}
	}
    Flash_eraseSector(addr);
    Filemanage_saveRecord();

    return;
}

// save file record
static void Filemanage_saveRecord(void)
{
    if(fileRecordAddr + sizeof(struct File_Pointer) > RECORD_SECTOR_ADDR + RECORD_SECTOR_SIZE)
    {
        Flash_eraseSector(RECORD_SECTOR_ADDR);
        fileRecordAddr = RECORD_SECTOR_ADDR;
    }
	else if(fileRecordAddr == RECORD_SECTOR_ADDR)
	{
		Flash_eraseSector(RECORD_SECTOR_ADDR);
	}
    // save the record
    Flash_writeData(fileRecordAddr, (uint8_t *)&fileRecord, sizeof(struct File_Pointer));
	#if READ_BACK_TEST
	Flash_readData(fileRecordAddr, (uint8_t *)&testBuf[0], sizeof(struct File_Pointer));
	#endif
	fileRecordAddr += sizeof(struct File_Pointer);
	
	
    return;
}
