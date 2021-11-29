// **** block and file storage related ****
#define	STORAGE_FILE_SIZE			1024
#define	STORAGE_MIN_IO_SIZE			7
#define	STORAGE_MAX_IO_SIZE			31
#define STORAGE_THREAD_COUNT		3

#define STORAGE_ODD_AND_EVEN		0
#define STORAGE_ODD					1
#define	STORAGE_EVEN				2

#define FILE_STORE_THREAD_LEN		1024
#define	STORAGE_WAIT_FOR_THREADS	(1024L * 3L)
#define STORAGE_HEAD_TAIL_LEN		32



int __stdcall	fileStorage	(
							void
							)

//	***************************************************************@@
//	- This function exercises file storage operations using three
//  threads.
//	*****************************************************************

{

#ifdef _CODE_DEBUG
//int					traceExecution = 1;						// for testing only
#endif

char					buffer[BUFSIZ],							// general purpose buffer
						errorString[BUFSIZ],					// error string
						path[BUFSIZ];							// file path

FILE_STORE_THREAD_ARG	*threadArg;								// thread argument

HANDLE					threadHandles[STORAGE_THREAD_COUNT];	// thread handle

int						lastError,								// last error
						retVal,									// returned by this function
						status;									// returned by function calls

longlong				fileSize;								// size of file to write

unsigned int			threadIDs[STORAGE_THREAD_COUNT];		// thread IDs

unsigned long			threadCount;							// number of threads to use

// **** initialization ****
retVal		= 0;												// hope all goes well

fileSize	= STORAGE_FILE_SIZE;								// for starters
lastError	= 0;												// for starters
threadArg	= NULL;												// for starters
threadCount = STORAGE_THREAD_COUNT;								// for starters

memset((void*)buffer,			(int)0x00, (size_t)sizeof(buffer));
memset((void*)errorString,		(int)0x00, (size_t)sizeof(errorString));
memset((void*)path,				(int)0x00, (size_t)sizeof(path));
memset((void*)threadHandles,	(int)0x00, (size_t)sizeof(threadHandles));

memset((void*)threadIDs,		(int)0x00, (size_t)sizeof(threadIDs));

// ???? ????
if (traceExecution != 0)
	{
	SysLog(EVENT_INFO, "fileStorage <<< sizeof(threadHandles): %lu line: %d\n", sizeof(threadHandles), __LINE__);
	SysLog(EVENT_INFO, "fileStorage <<<     sizeof(threadIDs): %lu line: %d\n", sizeof(threadIDs), __LINE__);
	}

// **** prompt for the path for the file(s) ****
strcpy(path, "c:\\temp");
printf(">>> specify a path [%s]: ", path);

// **** get the response from the user ****
if (fgets(buffer, BUFSIZ, stdin) == NULL)
	{
	SysLog(EVENT_ERROR,
	"fileStorage <<< fgets line: %d file ==>%s<==\n",
	__LINE__, __FILE__);
	retVal = WAR_INTERNAL_ERROR;
	goto done;
	}

// **** remove the CR ****
buffer[strlen(buffer) - 1] = '\0';
if (*buffer != '\0')
	strcpy(path, buffer);
else
	strcpy(path, "c:\\temp");

// ???? ????
SysLog(EVENT_INFO, "fileStorage <<< path ==>%s<== line: %d\n", path, __LINE__);

// **** prompt for the number of threads ****
printf(">>> number of threads / files [%lu]: ", threadCount);

// **** get the response from the user ****
if (fgets(buffer, BUFSIZ, stdin) == NULL)
	{
	SysLog(EVENT_ERROR, 
	"fileStorage <<< fgets line: %d file ==>%s<==\n",__LINE__, __FILE__);
	retVal = WAR_INTERNAL_ERROR;
	goto done;
	}

// **** remove the CR ****
buffer[strlen(buffer) - 1] = '\0';
if (*buffer != '\0')
	threadCount = atol(buffer);
threadCount = STORAGE_THREAD_COUNT;

// ???? ????
SysLog(EVENT_INFO, "fileStorage <<< threadCount: %lu line: %d\n", threadCount, __LINE__);

// **** loop starting a file storage thread at a time ****
for (int i = 0; i < threadCount; i++)
	{

	// **** allocate the thread argument ****
	threadArg = (FILE_STORE_THREAD_ARG*)calloc(	(size_t)1,
												(size_t)sizeof(FILE_STORE_THREAD_ARG));
	if (threadArg == (FILE_STORE_THREAD_ARG*)NULL)
		{
		SysLog(EVENT_ERROR,
		"TestQueInsert <<< calloc threadArg line: %d file ==>%s<==\n",
		__LINE__, __FILE__);
		retVal = -1;
		goto done;
		}

	// **** set the thread argument ****
	threadArg->fileSize = fileSize;
	strncpy(threadArg->path, path, strlen(path));

	// **** set the odd or even flag ****
	switch (i)
		{
		case 0:
			threadArg->oddOrEven = STORAGE_ODD_AND_EVEN;
		break;

		case 1:
			threadArg->oddOrEven = STORAGE_ODD;
		break;

		case 2:
			threadArg->oddOrEven = STORAGE_EVEN;
		break;

		default:
			SysLog(EVENT_ERROR,
			"fileStorage <<< UNEXPECTED i: %ld line: %d file ==>%s<==\n",
			status, __LINE__, __FILE__);
			retVal = status;
			goto done;
		break;
		}
	
	// **** start this thread ****
	threadHandles[i] = (HANDLE)_beginthreadex(	(void*)NULL,	// security descriptor
												(unsigned)0,	// stack size
												(SENCOR_THREAD_START)FileStoreThread,
												(void*)threadArg,

												0,				// running
												&threadIDs[i]);

	// **** check if something went wrong ****
	if (threadHandles[i] == (HANDLE)0)
		{
		strcpy(errorString, strerror(errno));
		SysLog(EVENT_ERROR,
		"fileStorage <<< _beginthreadex FileStoreThread errno: %d errorString ==>%s<== line: %d file ==>%s<==\n",
		errno, errorString, __LINE__, __FILE__);
		retVal = WAR_COULD_NOT_START_THREAD;
		goto done;
		}
	}

// **** wait for all threads to exit ****
status = WaitForMultipleObjects(sizeof(threadHandles) / sizeof(HANDLE),	// number of handles in array
								threadHandles,					// object-handle array
								(BOOL)(1 == 1),					// wait option
								STORAGE_WAIT_FOR_THREADS);		// time-out interval
switch (status)
	{
	case WAIT_OBJECT_0:
	case WAIT_OBJECT_0 + 1:
		SysLog(EVENT_INFO, "fileStorage <<< all threads have finished !!! line: %d\n", __LINE__);
	break;

	case WAIT_ABANDONED_0:
	case WAIT_FAILED:
	default:
		lastError = GetLastError();								// get error code
		PrintError(lastError);									// display string for error
		SysLog(EVENT_ERROR,
		"fileStorage <<< WaitForMultipleObjects status: %d lastError: %d line: %d file ==>%s<==\n", 
		status, lastError, __LINE__, __FILE__);
		retVal = status;
		goto done;
	break;
	}

// **** display `n` top and botton contents of each file ****
int n = STORAGE_HEAD_TAIL_LEN;

// ???? ????
SysLog(EVENT_INFO, "fileStorage <<< n: %d line: %d\n", n, __LINE__);

for (int i = 0; i < 3; i++)
	{
	status = displayContents(i, n);
	if (status != 0)
		{
		SysLog(EVENT_ERROR,
		"fileStorage <<< displayContents status: %d i: %d line: %d file ==>%s<==\n",
		status, i, __LINE__, __FILE__);
		retVal = status;
		goto done;
		}
	}

// **** clean up ****
done:

// **** inform the user what is going on ****
if (traceExecution != 0 || retVal != 0)
	SysLog(EVENT_INFO,
	"fileStorage <<< retVal: %d line: %d file ==>%s<==\n",
	retVal, __LINE__, __FILE__);

// **** inform the caller what went on ****
return retVal;
}



int __stdcall	FileStoreThread		(
									FILE_STORE_THREAD_ARG	*threadArg
									)

//	***************************************************************@@
//	- This thread populates a file with longlong numbers is ascending
//	order.
//	*****************************************************************

{

#ifdef _CODE_DEBUG
//int			traceExecution = 1;								// for testing only
#endif

char			fileName[BUFSIZ];								// file name

int				fd,												// file descriptor
				retVal,											// returned by this thread
				status;											// returned by function calls

longlong		data[STORAGE_MAX_IO_SIZE],						// data to write to file
				fileSize,										// size of file to write
				value;											// value to write to file

unsigned int	size;											// size of data to write to file

unsigned long	count,											// count of values to write
				threadID;										// our thread ID

// **** initialization ****
retVal			= 0;											// hope all goes well

count			= 0L;											// for starters
fd				= -1;											// for starters
size			= 0;											// for starters
status			= 0;											// for starters

threadID		= 0;											// for starters
value			= 0;											// for starters

memset((void*)data,		(int)0x00, (size_t)sizeof(data));
memset((void*)fileName,	(int)0x00, (size_t)sizeof(fileName));

// **** for ease of use ****
fileSize = threadArg->fileSize;

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "FileStoreThread <<< fileSize: %I64d line: %d\n", fileSize, __LINE__);

// **** get the thread ID ****
threadID = GetCurrentThreadId();

// **** generate the full path file name ****
status = sprintf(	fileName,
					"%s\\",
					threadArg->path);

// **** generate the file name ****
if (threadArg->oddOrEven == STORAGE_ODD_AND_EVEN)
	strcat(fileName, "a.bin");
else if (threadArg->oddOrEven == STORAGE_ODD)
	strcat(fileName, "b.bin");
else if (threadArg->oddOrEven == STORAGE_EVEN)
	strcat(fileName, "c.bin");
else
	{
	SysLog(EVENT_ERROR,
	"displayContents <<< UNEXPECTED threadArg->oddOrEven: %d line: %d file ==>%s<==\n",
	threadArg->oddOrEven, __LINE__, __FILE__);
	retVal = threadArg->oddOrEven;
	goto done;
	}

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "FileStoreThread <<< fileName ==>%s<== line: %d\n", fileName, __LINE__);

// **** remove output file (to start fresh) ****
status = _unlink(fileName);
if (status != 0 && errno != ENOENT)
	{
	SysLog(EVENT_ERROR,
	"FileStoreThread <<< _unlink status: %d fileName ==>%s<== errno: %d line: %d file ==>%s<==\n",
	status, fileName, errno, __LINE__, __FILE__);
	retVal = status;
	goto done;
	}

// **** create /open output file ****
fd = _open(	fileName,
			_O_CREAT | _O_RDWR | _O_BINARY | _O_EXCL,
			0777);
if (fd == -1)
	{
	SysLog(EVENT_ERROR,
	"FileStoreThread <<< _open fileName ==>%s<== errno: %d line: %d file ==>%s<==\n",
	fileName, errno, __LINE__, __FILE__);
	retVal = fd;
	goto done;
	}

// **** determine the firts value to store in the file ****
if (threadArg->oddOrEven == STORAGE_ODD)
	value = 1;
else
	value = 0;

// **** seed the random number generator ****
srand((unsigned)time(NULL));

// ***** loop writing data to the output file ****
for (longlong i = 0; i < fileSize; )
	{

	// **** number of values to write ****
	count = rand() % STORAGE_MIN_IO_SIZE + STORAGE_MIN_IO_SIZE;

	// ???? ????
	if (traceExecution != 0)
		SysLog(EVENT_INFO, "FileStoreThread <<< i: %I64d count: %lu line: %d\n", i, count, __LINE__);

	// **** populate array with values ****
	for (int j = 0; j < count; j++)
		{

		// **** set value in array ****
		data[j] = value;

		// **** increment value ****
		value++;
		if (threadArg->oddOrEven == STORAGE_ODD || threadArg->oddOrEven == STORAGE_EVEN)
			value++;
		}

	// **** write values to file ****
	size = count * sizeof(longlong);
	status = _write(fd,
					data,
					size);
	if (status != (int)size)
		{
		SysLog(EVENT_ERROR, 
		"FileStoreThread <<< _write status: %d size: %du errno: %d line: %d file ==>%s<==\n", 
		status, size, errno, __LINE__, __FILE__);
		retVal = status;
		goto done;
		}

	// **** increment i ****
	i += (longlong)count * sizeof(longlong);
	}

// **** clean up ****
done:

// **** close output file ****
if (fd != -1)
	{
	status = _close(fd);
	if (status != 0)
		{
		SysLog(EVENT_ERROR, 
		"FileStoreThread <<< _close fd: %d errno: %d line: %d file ==>%s<== \n",
		fd, errno, __LINE__, __FILE__);
		retVal = -1;
		}
	}

// **** free the thread argument ****
if (threadArg != (FILE_STORE_THREAD_ARG*)NULL)
	free((void*)threadArg);

// **** inform the user what is going on ****
if (traceExecution != 0 || retVal != 0)
	SysLog(EVENT_INFO,
	"FileStoreThread <<< threadID: %ld retVal: %d line: %d file ==>%s<==\n",
	threadID, retVal, __LINE__, __FILE__);

// **** allow other threads to execute ****
SLOW_DOWN;

// **** exit this thread ****
_endthread();
}



int __stdcall	displayContents	(
								int		i,
								int		n
								)

//	***************************************************************@@
//	- This function dumps the specified number of lines on the top
//  and bottom of the associated file.
//	*****************************************************************

{

#ifdef _CODE_DEBUG
//int			traceExecution = 1;								// for testing only
#endif

char			errorString[BUFSIZ],							// error string
				fileName[BUFSIZ];								// full path of file to be displayed

int				fd,												// file descriptor
				retVal,											// returned by this thread
				status;											// returned by function calls

longlong		*data;											// to read from file

struct _stati64	statBuffer;										// file status data structure

unsigned long	bytesRead,										// number of bytes read
				bytesToRead,									// number of bytes to read
				offset;											// file offset

// **** initialization ****
retVal = 0;														// hope all goes well

data	= NULL;													// for starters
fd		= -1;													// for starters
offset	= (unsigned long)0L;									// for starters

memset((void*)errorString,	(int)0x00, (size_t)sizeof(errorString));
memset((void*)fileName,		(int)0x00, (size_t)sizeof(fileName));
memset((void*)&statBuffer,	(int)0x00, (size_t)sizeof(statBuffer));

// **** compute number of bytes to read ****
bytesToRead = n * sizeof(longlong);

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "displayContents <<< bytesToRead: %lu line: %d\n", bytesToRead, __LINE__);

// **** allocate data buffer ****
data = (longlong*)calloc(	(size_t)1,
							(size_t)bytesToRead);
if (data == (longlong*)NULL)
	{
	SysLog(EVENT_ERROR,
	"displayContents <<< calloc bytesToRead: %lu line: %d file ==>%s<==\n",
	bytesToRead, __LINE__, __FILE__);
	retVal = -1;
	goto done;
	}

// **** generate name of file ****
if (i == STORAGE_ODD_AND_EVEN)
	status = sprintf(fileName, "c:\\temp\\a.bin");
else if (i == STORAGE_ODD)
	status = sprintf(fileName, "c:\\temp\\b.bin");
else if (i == STORAGE_EVEN)
	status = sprintf(fileName, "c:\\temp\\c.bin");
else
	{
	SysLog(EVENT_ERROR,
	"displayContents <<< UNEXPECTED i: %d line: %d file ==>%s<==\n",
	i, __LINE__, __FILE__);
	retVal = i;
	goto done;
	}

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "displayContents <<< fileName ==>%s<== line: %d\n", fileName, __LINE__);

// **** open file of interest ****
fd = _open(	fileName,
			_O_RDONLY | _O_BINARY | _O_EXCL);
if (fd == -1)
	{
	SysLog(EVENT_ERROR,
	"displayContents <<< _open fileName ==>%s<== errno: %d line: %d file ==>%s<==\n",
	fileName, errno, __LINE__, __FILE__);
	retVal = errno;
	goto done;
	}

// **** display header info ****
printf("displayContents <<< contents of fileName ==>%s<== follow:\n", fileName);

// **** read top n lines ****
bytesRead = _read(	fd,
					data,
					bytesToRead);								// number of bytes read
if (bytesRead != bytesToRead)
	{
	SysLog(EVENT_ERROR,
	"displayContents <<< _read bytesRead: %lu fileName ==>%s<== errno: %d line: %d file ==>%s<==\n",
	bytesRead, fileName, errno, __LINE__, __FILE__);
	retVal = errno;
	goto done;
	}

// **** loop displaying top n lines ****
for (int i = 1; i <= n; i++)
	{

	// **** ****
	printf("%I64d ", data[i - 1]);

	// **** line separator ****
	if (i != 0 && i % 8 == 0)
		printf("\n");
	}

// **** terminate last line ****
if (n % 8 != 0)
	printf("\n");

// ***** get the size of the file ****
status = _stati64(	fileName,
					&statBuffer);
if (status != 0)
	{
	strcpy(errorString, strerror(errno));
	EventLog(EVENT_ERROR,
	"displayContents <<< _stati64 fileName ==>%s<== errno: %d errorString ==>%s<== line: %d file ==>%s<==\n",
	fileName, errno, errorString, __LINE__, __FILE__);
	retVal = errno;
	goto done;
	}

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "displayContents <<< statBuffer.st_size: %I64d line: %d\n", statBuffer.st_size, __LINE__);

// **** seek to the start of the bottom n lines in this file ****
offset = _lseek(fd,
				-bytesToRead,
				SEEK_END);
if (offset == (unsigned long)-1L)
	{
	strcpy(errorString, strerror(errno));
	EventLog(EVENT_ERROR,
	"displayContents <<< _lseek offset: %ld bytesToRead: %ld errno: %d errorString ==>%s<== line: %d file ==>%s<==\n", 
	offset, (long)bytesToRead, errno, errorString, __LINE__, __FILE__);
	retVal = status;
	goto done;
	}

// ???? ????
if (traceExecution != 0)
	SysLog(EVENT_INFO, "displayContents <<< offset: %lu fileName ==>%s<== line: %d\n", offset, fileName, __LINE__);

// **** read botton n values ****
bytesRead = _read(	fd,
					data,
					bytesToRead);								// number of bytes read
if (bytesRead != bytesToRead)
	{
	strcpy(errorString, strerror(errno));
	SysLog(EVENT_ERROR,
	"displayContents <<< _read bytesRead: %lu bytesToRead: %lu fileName ==>%s<== errno: %d errorString ==>%s<== line: %d file ==>%s<==\n",
	bytesRead, bytesToRead, fileName, errno, errorString, __LINE__, __FILE__);
	retVal = errno;
	goto done;
	}

// **** display separator line ****
printf(":::: :::: :::: ::::\n");

// **** loop displaying bottom n lines ****
for (int i = 1; i <= n; i++)
	{

	// **** display value ****
	printf("%I64d ", data[i - 1]);

	// **** line separator ****
	if (i != 0 && i % 8 == 0)
		printf("\n");
	}

// **** terminate last line ****
if (n % 8 != 0)
	printf("\n");

// **** clean up ****
done:

// **** close output file ****
if (fd != -1)
	{
	status = _close(fd);
	if (status != 0)
		{
		SysLog(EVENT_ERROR,
		"displayContents <<< _close fd: %d errno: %d line: %d file ==>%s<== \n",
		fd, errno, __LINE__, __FILE__);
		retVal = -1;
		}
	}

// **** free the data buffer ****
if (data != (longlong*)NULL)
	free((void*)data);

// **** inform the user what is going on ****
if (traceExecution != 0 || retVal != 0)
	SysLog(EVENT_INFO,
	"displayContents <<< retVal: %d line: %d file ==>%s<==\n",
	retVal, __LINE__, __FILE__);

// **** inform the caller what went on ****
return retVal;
}



int __cdecl	main	(
					int		argc,
					char	*argv[]
					)

//	***************************************************************@@
//	- Test the operation of a specifiec function.
//	*****************************************************************

{

int     retVal,									            // returned by this function
        status;									            // returned by function calls

// **** initialization ****
retVal  = 0;												// hope all goes well

status = fileStorage();
if (status != 0)
    {
    SysLog(EVENT_ERROR,
    "main <<< fileStorage status: %d line: %d file ==>%s<==\n",
    status, __LINE__, __FILE__);
    retVal = status;
    goto done;
    }  

// **** clean up ****
done:

// **** inform user what went going on ****
if (traceExecution != 0 || retVal != 0)
	SysLog(EVENT_INFO,
	"main <<< retVal: %d line: %d file ==>%s<==\n",
	retVal, __LINE__, __FILE__);

// **** inform caller what went on ****
return retVal;
}