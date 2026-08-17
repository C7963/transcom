//#include "stdafx.h"
#include "pch.h"
#include "XillyFile.h"
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys\types.h>
#include <share.h>

/*
 * 定义一些错误码，用于扩充现有的 errno
 * 暂时没有判断这些错误码是否和现有的有冲突
*/
#define FILE_E_INPERR		 -100       /* 输入参数有误                      */
#define FILE_E_WRITERET0     -101       /* write() 函数返回值为0             */
#define FILE_E_FILENOOPEN    -102       /* 文件未被打开                      */
#define FILE_E_FDSETFailed   -103       /* FD_ISSET() 连续失败次数超过最大数量 */

/*
 * 构造函只进行文件名存储，暂不做任何操作
*/
CommBus::XillyFile::XillyFile(std::string filePathStr)
{
	m_filePath = filePathStr;
	m_fd = -1;
	m_isOpened = false;
	m_currentOpenFlag = O_RDONLY;
	m_offsetVal = 0;
	m_errno = 0;
	m_singleWriteDataMaxLen = 65536;
	m_isWriteOneTime = false;
	m_writtenDataLen = 0;
	m_singleReadDataMaxLen = 4096;
	m_isReadOneTime = false;
	m_readedDataLen = 0;
	m_isReadFileEnd = false;
	reset_errno();
}

CommBus::XillyFile::~XillyFile()
{
	close_file();
}

void CommBus::XillyFile::reset_errno()
{
	m_errno = 0;
}

bool CommBus::XillyFile::open_file(DeviceFileOpenType openTypeVal)
{
	switch (openTypeVal) {
	case DeviceFileOpenType::e_ReadOnly:  return open_file(_O_RDONLY | _O_BINARY);
	case DeviceFileOpenType::e_WriteOnly: return open_file(_O_WRONLY | _O_BINARY);
	case DeviceFileOpenType::e_ReadWrite: return open_file(_O_RDWR | _O_BINARY);
	default:						return false;
	}
}

/*
 * 打开文件，打开方式由 openFlagVal 决定
 * openFlagVal 取值请参考 open() 函数的第二个参数
 *
 * 打开成功则返回 true，
 * 打开失败则返回 false，同时将 errno 保存到 m_errno 中
*/
bool CommBus::XillyFile::open_file(int openFlagVal)
{
	reset_errno();

	/*
	 * 如果文件已经被打开，则判断入参的打开方式和当前的打开方式是否一样，
	 * 如果不一样，则需要按照新的打开方式重新打开；如果一样则不做任何操作。
	*/
	if (m_isOpened && m_currentOpenFlag == openFlagVal)
	{
		return true;
	}
	if (m_isOpened && m_currentOpenFlag != openFlagVal)
	{
		if (!(close_file()))
		{
			return false;
		}
	}

	m_currentOpenFlag = openFlagVal;

	int fd_temp = 0;
	m_errno = _sopen_s((int*)(&fd_temp), m_filePath.c_str(), m_currentOpenFlag, _SH_DENYRW, _S_IREAD);
	if (m_errno != 0)
	{
		/*std::cout << "Failed to open file: " << m_filePath << "! "
			<< "Error Code(" << m_errno << "): " << get_now_errnoInfo_str() << "."
			<< std::endl;*/
		return false;
	}
	m_fd = fd_temp;
	m_isOpened = true;
	return true;
}

/*
 * 关闭文件操作
 * 调用 close() 函数进行文件关闭操作，
 * 执行成功则返回 true;
 * 执行失败则返回 false，同时将 errno 保存到 m_errno 中
 *
 * 注意： 当前代码中，调用 close() 函数失败时也会
 *       重置 m_fd 和 m_isOpened 两个成员变量，
 *       所以当 close_file() 调用失败后无法再次
 *       调用 close_file() 去关闭未成功关闭的文
 *       件（当前的做法就是，若关闭失败，那就不管了），
 *       暂时还未找到一个合理的解决方案
*/
bool CommBus::XillyFile::close_file()
{
	try {
		reset_errno();
		//如果文件已经关闭则直接返回
		if (!m_isOpened)
		{
			return true;
		}
		int ret = 0;
		if (m_fd >= 0) {
			ret = _close(m_fd);
			//关闭文件时出错，则保存错误码
			if (ret != 0)
			{
				m_errno = errno;
				std::cout << "Failed to close file: " << m_filePath << "! "
					<< "Error Code(" << m_errno << "): " << get_now_errnoInfo_str() << "."
					<< std::endl;
			}
			m_fd = -1;
			m_isOpened = false;
		}
		return (ret == 0) ? true : false;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

/*
 * 设置文件内的文件指针偏移量，
*/
bool CommBus::XillyFile::set_offset(uint64_t offsetVal)
{
	reset_errno();
	try
	{
		//_off64_t ret = lseek64(m_fd, offsetVal, SEEK_SET);
		uint64_t ret = _lseeki64(m_fd, offsetVal, SEEK_SET);
		if (ret == -1)
		{
			m_errno = errno;

			std::cout << "lseek() set file: " << m_filePath << " offset " << m_offsetVal << " failed! "
				<< "Error Code(" << m_errno << "): " << get_now_errnoInfo_str() << "."
				<< std::endl;
			return false;
		}
	}
	catch (const std::exception&)
	{
		return false;
	}
	
	return true;
}

/*
 * 将数据 dataBuf 写入当前的文件中，数据长度为 bufLen
 *
 * 返回值：bool 值    true 表示操作成功，即写数据时没有错误，但是不能表示
 *                        数据已经全部写入文件中（虽然从现有代码逻辑来看，
 *                        返回 true 的时候数据肯定已经全部写入文件），若
 *                        要获取已经写入的数据长度，可以调用 written_dataLen() 函数查看
 *                  false 表示操作失败，即写数据时发生了某种错误，这时会
 *                        将 errno 保存到 m_errno 中，可以调用 get_errno() 函数查看当前的 errno 值
 *
 * 注意：1.在调用本函数前请先确保文件已经被成功打开（调用 open_file()函数），
 *        本函数不会自动调用 open_file() 函数;
 *      2.本函数使用时一般和 written_dataLen() 配合使用;
 *      3.本函数如果写入字符串数据，字符串最后的 '\0' 字符是否写入需要用户自己根据当前的情况判断，
 *        若将 '\0' 写入文件，则有可能会破坏文件的解析方式（若原来是文本文件，则写入后该文件将无法
 *        再被当做文本文件来解析，会被解析成二进制文件），正常情况下，字符串最后的 '\0' 是不会被写
 *        入文件的（如函数 fprint() ）,所有在调用本函数写入字符串时，一般不写如最后的 '\0',除非有
 *        特殊情况。
*/
bool CommBus::XillyFile::write_data(uint8_t* dataBuf, uint32_t bufLen)
{
	reset_errno();
	//m_writtenDataLen = 0;
	if (bufLen <= 0) {
		m_errno = FILE_E_INPERR;
		return false;
	}
	////复制传入的数据
	/*uint8_t* dataBufTemp = new uint8_t[bufLen];
	memset(dataBufTemp, 0, bufLen * sizeof(uint8_t));
	std::cout << "SendData" << std::endl;*/
	//if (bufLen != 8192)
		/*for (int j = 0; j < bufLen; j += 4)
			for (uint32_t i = j; i < j + 4; i++)
			{
				dataBufTemp[j + j + 3 - i] = dataBuf[i];
			}*/
	//else
		/*for (int j = 0; j < bufLen; j++)
			dataBufTemp[j] = dataBuf[j];*/
			
	/*
	 * 若用户需要一次性写入所有数据（m_isWriteOneTime 为 true），则直接将 singleWriteDataMaxLenTemp 设置为 bufLen;
	 * 否则将 singleWriteDataMaxLenTemp 设置为 m_singleWriteDataMaxLen，即分多次进行数据写入操作（可能分多次），每次
	 * 最多写入 singleWriteDataMaxLenTemp 个字节的数据。
	*/
	/*uint32_t singleWriteDataMaxLenTemp = m_isWriteOneTime ? bufLen : m_singleWriteDataMaxLen;*/

	uint32_t needWriteDataLen = 0;    //每次写数据时，实际需要写入的长度
	uint32_t dataBufWritedLen = 0;    //每次成功写入数据后，已经写完的总的数据长度
	bool     isWriteDataSucceed = true; //数据写入是否成功
	int      writeRet = 0;

	/*
	 * eintrErrTime 用以记录连续出现 errno == EINTR 情况的次数;
	 * 当 eintrErrTime 大于 eintrErrMaxTime 时则终止写入操作
	*/
	int       eintrErrTime = 0;
	const int eintrErrMaxTime = 50;

	while (true)
	{
		reset_errno();
		if (!m_isOpened) {
			m_errno = FILE_E_FILENOOPEN;
			isWriteDataSucceed = false;
			break;
		}
		needWriteDataLen = bufLen - dataBufWritedLen;
		if (needWriteDataLen <= 0)
			break;
		writeRet = _write(m_fd, (dataBuf + dataBufWritedLen), needWriteDataLen);
		if (writeRet > 0)
		{
			eintrErrTime = 0;
			if ((uint32_t)writeRet != needWriteDataLen) {
				std::cout << "Written data omission. File name: " << m_filePath << "." << std::endl;
			}
			dataBufWritedLen += writeRet;
			if (dataBufWritedLen >= bufLen) break;
		}
		else if (writeRet < 0)
		{
			m_errno = errno;
			if (m_errno == EINTR)
			{
				std::cout << "write(): " << m_filePath << " failed by interrupted system call. "
					<< "Error Code(" << m_errno << "): " << get_now_errnoInfo_str() << "."
					<< std::endl;
				//QThread::msleep(10);
				eintrErrTime++;
				if (eintrErrTime < eintrErrMaxTime) { continue; }
				else { isWriteDataSucceed = false; break; }
			}
			else
			{
				eintrErrTime = 0;
				std::cout << "write(): " << m_filePath << " failed. "
					<< "Error Code(" << m_errno << "): " << get_now_errnoInfo_str() << "."
					<< std::endl;
				isWriteDataSucceed = false;
				break;
			}
		}
		else if (writeRet == 0)
		{
			eintrErrTime = 0;
			m_errno = FILE_E_WRITERET0;
			std::cout << "writer() function returns 0(?!): " << m_filePath << std::endl;
			isWriteDataSucceed = false;
			break;
		}
	}
	//m_writtenDataLen = dataBufWritedLen;
	//delete[] dataBufTemp;
	return isWriteDataSucceed;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint32_t CommBus::XillyFile::written_dataLen()
{
	return m_writtenDataLen;
}

/*
 * 从文件中读取指定 bufLen 长度的数据，并保存到出参 dataBufOut 中
 *
 * 返回值：bool 值    true 表示操作成功，即读数据时没有错误，但是不能表示
 *                        已经读取到指定长度的数据，若要获取已经读取的数
 *                        据长度，可以调用 readed_dataLen() 函数查看，
 *                        同时也可以调用 isReadEOF() 函数查看是否已经
 *                        读到文件末尾;
 *                  false 表示操作失败，即写数据时发生了某种错误，这时会
 *                        将 errno 保存到 m_errno 中，可以调用 get_errno()
 *                        函数查看当前的 errno 值
 *
 * 注意：1.在调用本函数前请先确保文件已经被成功打开（调用 open_file()函数），
 *        本函数不会自动调用 open_file() 函数;
 *      2.本函数使用时一般和 readed_dataLen() 及 isReadEOF() 配合使用;
 *        数据完全成功读取的条件为 函数返回值为 true，同时 readed_dataLen() 和 bufLen 相等
*/
// 【推荐修改版】返回实际读取的字节数 + 错误码分离
uint32_t CommBus::XillyFile::read_data(uint8_t* dataBufOut, uint32_t bufLen)
{
	reset_errno();
	m_isReadFileEnd = false;
	m_readedDataLen = 0;        // 这个成员变量保留，但我们主要靠返回值

	if (!dataBufOut || bufLen == 0) {
		m_errno = FILE_E_INPERR;
		return 0;
	}

	if (!m_isOpened) {
		m_errno = FILE_E_FILENOOPEN;
		return 0;
	}

	uint32_t totalRead = 0;
	uint8_t* ptr = dataBufOut;
	const int MAX_EINTR_RETRY = 50;

	while (totalRead < bufLen) {
		uint32_t want = bufLen - totalRead;

		int ret = _read(m_fd, ptr + totalRead, want);

		if (ret > 0) {
			totalRead += ret;
			if (totalRead >= bufLen)
				break;
			continue;  // 继续读剩下的
		}

		if (ret == 0) {  // EOF
			m_isReadFileEnd = true;
			break;
		}

		// ret < 0
		m_errno = errno;

		if (errno == EINTR) {
			static int eintrCount = 0;
			if (++eintrCount < MAX_EINTR_RETRY) {
				continue;
			}
			std::cerr << "[XillyFile] Too many EINTR, give up: " << m_filePath << std::endl;
			return totalRead;  // 返回已读部分
		}

		// 其他错误（如 EAGAIN, EIO 等）
		std::cerr << "[XillyFile] read() failed: " << m_filePath
			<< " | errno=" << errno << " | " << strerror(errno) << std::endl;
		break;  // 出错就退出
	}

	m_readedDataLen = totalRead;  // 保留旧行为兼容
	return totalRead;             // 关键：返回实际读取的字节数！
}uint32_t CommBus::XillyFile::readed_dataLen()
{
	return m_readedDataLen;
}
bool CommBus::XillyFile::isReadEOF()
{
	return m_isReadFileEnd;
}
/*
 * 获取错误码 errno 对应的说明
*/
std::string CommBus::XillyFile::get_errnoInfo_str(errno_t errnoVal)
{
	std::string infoStr;
	switch (errnoVal)
	{
	case FILE_E_INPERR:
		infoStr = "The input parameter has an error.";
		break;
	case FILE_E_WRITERET0:
		infoStr = "The write() function retuen val was 0.";
		break;
	case FILE_E_FILENOOPEN:  infoStr = "The file has been closed.";
		break;
	case FILE_E_FDSETFailed: infoStr = "The FD_ISSET() function execute failed";
		break;
	default:
	{
		char errInfoTemp[256] = { 0 };
		errno_t errVal = strerror_s(errInfoTemp, 256, m_errno);
		if (errVal != 0)
			infoStr = "Get Error info failed!";
		else
			infoStr = std::string(errInfoTemp);
	}
	}

	return infoStr;
}