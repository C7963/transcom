#pragma once
/*
 * 封装一些 C 方式的文件操作方法
 * 主要封装 open, close, read, write ，lseek 及 access 函数
 *
 * 注意：1. 本类是线程不安全的，在多线程下进行操作时需要多加注意
 *
 * 暂时存在的问题：
 *      1. 在对 Xillybus 产生的设备文件进行读操作时，有可能会卡死，
 *         产生这种情况的原因为
 *              上位机在进行读操作的时候，调用系统 read() 函数开始读数据，
 *              底层硬件接收到 read() 函数命令后开始准备数据，这样上位机
 *              就能读到数据，一直读到上位机需要的数据长度时就会停止读取操
 *              作；但是当上位机需要读取的数据很长，同时底层硬件无法准备这
 *              么多数据时，此时上位机将会被卡死（open() 默认是以阻塞方式
 *              打开文件的，如果以非阻塞方式打开文件，读写操作将不可预计，
 *              已测试过），因为还没有读到期望长度数据。通常防止read() 函
 *              数卡死的方式是结合 select() 函数一起使用，但是由于底层硬
 *              件的原因（底层硬件只有在接收到 read() 函数命令后才开始准备
 *              数据，但是 select() 函数只有检测到底层数据准备好之后才调用
 *              read() 函数，这里就产生了一个矛盾，暂时无法解决），此方式没
 *              有产生预期的效果。
 *      2. 当关闭文件时由于某些问题导致文件无法正常关闭的，此情形暂时无法解决。
 *         需后期考虑修复。
*/

#include <string>
#include <stdint.h>
namespace CommBus
{
	class __declspec (dllexport)XillyFile
	{
		/*
		* 用枚举定义文件打开的方式
		*/

	public:
		enum DeviceFileOpenType {
			e_ReadOnly = 0, //以只读的方式打开文件
			e_WriteOnly = 1,     //以只写的方式打开文件
			e_ReadWrite = 2
		};
		XillyFile(std::string XillyFile); // 若 filePathStr 表示的路径中包含 '\' 字符，请使用转义字符 '\\' 表示
		~XillyFile();
		std::string get_filePath() { return m_filePath; }

		bool open_file(DeviceFileOpenType openTypeVal);  //打开文件
		bool close_file();                         //关闭文件
		bool is_opened() { return m_isOpened; }    //获取文件是否已经被打开
		bool set_offset(uint64_t offsetVal);       //设置文件内指针偏移量，偏移量 offsetVal 相对于文件开头的位置
		/*******************************************************************************************/
		uint32_t singleWriteDataMaxLen() { return m_singleWriteDataMaxLen; }
		void     set_singleWriteDataMaxLen(uint32_t val) { if (val <= 0) return; m_isWriteOneTime = false; m_singleWriteDataMaxLen = val; } //在写数据时，设置程序每次写入数据时的最大长度

		bool isWriteOneTime() { return m_isWriteOneTime; }
		void set_writeDataOneTime() { m_isWriteOneTime = true; }        //在写数据时，设置程序进行一次性写入操作

		bool     write_data(uint8_t* dataBuf, uint32_t bufLen);
		uint32_t written_dataLen();                                     // 获取实际写入文件的数据长度，该函数配合 write_data() 一起使用，单独使用无意义
		/*******************************************************************************************/

		uint32_t singleReadDataMaxLen() { return m_singleReadDataMaxLen; }
		void     set_singleReadDataMaxLen(uint32_t val) { if (val <= 0) return; m_isReadOneTime = false; m_singleReadDataMaxLen = val; } //在读取数据时，设置程序每次读取数据时的最大长度

		bool isReadOneTime() { return m_isReadOneTime; }
		void set_readDataOneTime() { m_isReadOneTime = true; }          //在读取数据时，设置程序进行一次性读取操作

		uint32_t read_data(uint8_t* dataBufOut, uint32_t bufLen);
		uint32_t readed_dataLen();                                      // 获取实际已经从文件中读取的数据长度，该函数配合 read_data() 一起使用，单独使用无意义
		bool     isReadEOF();                                           // 是否已经读到文件末尾了，该函数配合 read_data() 一起使用，单独使用无意义

		/*******************************************************************************************/
		errno_t get_errno() { return m_errno; }													//获取当前错误码
		std::string get_errnoInfo(errno_t errnoVal) { return get_errnoInfo_str(errnoVal); }    //获取错误码对应的说明信息
		std::string get_now_errnoInfo() { return get_errnoInfo(m_errno); }									//获取当前错误码对应的说明信息
	private:
		std::string	 m_filePath;
		int      m_fd;
		bool     m_isOpened;        //文件是否已经被打开
		int      m_currentOpenFlag; //当前文件打开的形式，参数取值参考 open() 函数的第二个参数
		uint64_t m_offsetVal;       //设置文件内的文件指针偏移量，取值参考 lseek() 函数
		errno_t  m_errno;           //最近一次操作失败的 errno

		/*******************************************************************************************/

		/*
		 * 当调用 write_data() 写数据时，如果数据特别长，数据有可能是分多次写下去的，该变量表示每次最多能写入的字节数
		*/
		uint32_t m_singleWriteDataMaxLen;
		/*
		 * 当调用 write_data() 写数据时，是否一次性写入所有数据，true表示一次性写入所有数据，false表示可以分多次写入，
		 * 默认为 false， 即可以进行多次写入。
		 * 该变量与 m_singleWriteDataMaxLen 为互斥条件，当本变量为 true 时 m_singleWriteDataMaxLen 不可用，当用户设置
		 * m_singleWriteDataMaxLen 时，本变量将被自动赋值为 false
		*/
		bool     m_isWriteOneTime;
		/*
		 * 每当调用 write_data() 进行数据写入操作时，用来存储实际写入文件的数据长度;
		 * 若数据写入成功，则 m_writtenDataLen 为用户设置的值（ write_data() 函数的第二个行参）；
		 * 若数据写入失败，即数据没有全部写入文件，则 m_writtenDataLen 为实际写入的长度
		*/
		uint32_t m_writtenDataLen;

		/*******************************************************************************************/

		/*
		 * 当调用 read_data() 读数据时，如果需要读取的数据特别长，数据有可能是分多次读上来的，该变量表示每次能读取的最多的字节数
		*/
		uint32_t m_singleReadDataMaxLen;
		/*
		 * 当调用 read_data() 读数据时，是否一次性读取所需的数据长度，true表示一次性读取所需的数据长度，false表示可以分多次读取，
		 * 默认为 false， 即可以进行多次读取。
		 * 该变量与 m_singleReadDataMaxLen 为互斥条件，当本变量为 true 时，m_singleReadDataMaxLen 不可用，当用户设置
		 * m_singleReadDataMaxLen 时，本变量将被自动赋值为 false
		*/
		bool     m_isReadOneTime;
		/*
		 * 每当调用 read_data() 进行数据读取操作时，用来存储实际读取到的数据长度;
		 * 若数据读取成功，则 m_readedDataLen 为用户设置的值（ read_data() 函数的第二个行参）；
		 * 若数据读取失败，即数据没有读到指定长度，则 m_readedDataLen 为实际读取的长度
		*/
		uint32_t m_readedDataLen;
		bool     m_isReadFileEnd; //当对文件进行读取操作时，用于标记是否已经读到文件末尾
		/*******************************************************************************************/
		bool open_file(int openFlagVal);
		void reset_errno();
		std::string get_errnoInfo_str(errno_t errnoVal);                       //获取错误码对应的说明信息
		std::string get_now_errnoInfo_str() { return get_errnoInfo_str(m_errno); } //获取当前错误码对应的说明信息
	};
}
