/**
* @file myfind.c
* Betriebssysteme Beispiel 1
* myfind - vereinfachte Version des Linux Befehls "find"
*
* @author Ralf Ziefuhs <ic17b065@technikum-wien.at>
* @author Clemens Fritzsche <ic17b087@technikum-wien.at>
*
* @date 2018/04/06
*
* @version 1
*
*/

/*
* -------------------------------------------------------------- includes --
*/
#include <dirent.h> // readdir, opendir
#include <stdio.h> // fprintf
#include <errno.h> // errno
#include <sys/stat.h> // lstat, stat
#include <string.h> // strcmp
#include <pwd.h> // getpwnam, getpwuid
#include <limits.h> //
#include <libgen.h> //basename
#include <fnmatch.h> // fnmatch
#include <stdbool.h> // type bool, true, false
#include <unistd.h> // readlink
#include <stdlib.h> // calloc, free
#include <time.h> // localtime
#include <grp.h> //getgrgid
/*
* --------------------------------------------------------------- defines --
*/
#define SUCCESS 0
#define ERROR 1
/*
* -------------------------------------------------------------- typedefs --
*/
enum Bool{ FALSE, TRUE };
/*
* ------------------------------------------------------------- functions --
*/
static void do_file(const char * file_name, const char * const * parms, const int offset);
static void do_dir(const char * dir_name, const char * const * parms, const int offset);
static int do_print(const char * file_name, const char * const * parms);
static int do_check_parms(const char * const * parms);
static int do_user(const struct stat buffer, const char * const * parms, const int offset);
static int do_name(const char * file_name, const char * const * parms, const int offset);
static int do_ls(const struct stat buffer, const char * file_name, const char * const * parms);
static int do_nouser(const struct stat buffer, const char * const * parms);
static int do_type(const struct stat buffer, const char * const * parms, const int offset);
static char get_file_type(const mode_t mode);
static int do_path(const char * file_name, const char * const * parms, const int offset);


/**
* \brief The simplified version of "find"
*
* This is the main entry point of the programm myfind
*
* \param argc the number of arguments
* \param argv the arguments (parameters) inclusively the program name in argv[0]
*
* \return always "SUCCESS"
* \retval 0 always
*/
int main(int argc, const char const *argv[])
{
	if (argc > 1) //checks if there are arguments/parameters in the command line
	{		
		//check if the prompted parameter are correct
		//if wrong then EXIT
		if (do_check_parms(argv) == ERROR)
		{
			/* ### FB not really an issue, but find doesn't print usage if a user was not found or, an option misses an argument and usage should contain only one language*/
			fprintf(stderr, "Usage: %s\t<file or directory> [ <test-aktion> ] ...\n"
				"-user <name/uid>\n"
				"-name <pattern>\n"
				"-type [bcdpfls]\n"
				"-print\n"
				"-ls\n"
				"-nouser\n"
				"-path <pattern>\n",
				argv[0]);

			exit(EXIT_FAILURE);
		}

		do_file(argv[1], argv, 2);
	}
	/* ### FB you could also write a do_print_usage() function to not have to debug two very similar looking code segments, if for instance you added more functionality*/
	else //prints out the correct usage
	{
		fprintf(stderr, "Usage: %s\t<file or directory> [ <test-aktion> ] ...\n"
			"-user <name/uid>\n"
			"-name <pattern>\n"
			"-type [bcdpfls]\n"
			"-print\n"
			"-ls\n"
			"-nouser\n"
			"-path <pattern>\n",
			argv[0]);

		exit(EXIT_FAILURE);
	}

	if (fflush(stdout) == EOF) //flushes the stdout buffer
	{
		fprintf(stderr, "%s Unable to flush stdout: \n", strerror(errno));
	}
	return EXIT_SUCCESS;

}


/**
*
* \name do_file
*
* \brief The function checks the file and calls do_print() for a file if it matches the
* added parameters. Additionally it calls do_dir() if the file is a directory
*
* \param file_name		full path of the file to check
* \param parms			complete parameter list
* \param offset			value of the offset to the previous parameter/filter
*
* \return nothing
*
*/
static void do_file(const char *file_name, const char * const * parms, const int offset)
{
	struct stat buf;
	int temp_offset = offset;
	int check_action = SUCCESS;
	int print_done = FALSE;
	
	if (lstat(file_name, &buf) == -1)
	{
		fprintf(stderr, "%s: `%s' %s\n", *parms, file_name, strerror(errno));
		return;
	}
		
	//runs as long as there are commandline arguments and the check
	//of the file with the prompted parameters is successful
	while (*(parms + temp_offset) != NULL && check_action == SUCCESS)
	{
		if (strcmp(*(parms + temp_offset), "-user") == 0)
		{
			check_action = do_user(buf, parms, (temp_offset + 1));
			temp_offset = temp_offset + 2;
			continue;
		}

		if (strcmp(*(parms + temp_offset), "-name") == 0)
		{
			check_action = do_name(file_name, parms, (temp_offset + 1));
			temp_offset = temp_offset + 2;
			continue;
			
		}
		if (strcmp(*(parms + temp_offset), "-type") == 0)
		{
			check_action = do_type(buf, parms, (temp_offset + 1));
			temp_offset = temp_offset + 2;
			continue;
			
		}
		if (strcmp(*(parms + temp_offset), "-print") == 0)
		{
			check_action = do_print(file_name, parms);
			temp_offset = temp_offset + 1;
			print_done = TRUE;
			continue;
		}
		if (strcmp(*(parms + temp_offset), "-ls") == 0)
		{
			check_action = do_ls(buf, file_name, parms);
			temp_offset = temp_offset + 1;
			print_done = TRUE;
			continue;
			
		}
		if (strcmp(*(parms + temp_offset), "-nouser") == 0)
		{
			check_action = do_nouser(buf, parms);
			temp_offset = temp_offset + 1;
			continue;
		}
		if (strcmp(*(parms + temp_offset), "-path") == 0)
		{
			check_action = do_path(file_name, parms, (temp_offset + 1));
			temp_offset = temp_offset + 2;
			continue;
			
		}
	}

	//do_print() if the file is not printed until now and the check was successful
	if (print_done == FALSE && check_action == SUCCESS)
	{
		do_print(file_name, parms);
	}

	if (S_ISDIR(buf.st_mode))
	{
		do_dir(file_name, parms, offset);
	}

	return;
}


/**
*
* \name do_dir
*
* \brief The function reads the entries of the directory for files
*
* When a file is found the function calls do_file()
*
* \param dir_name		full path of the directory
* \param parms			complete parameter list
* \param offset			value of the offset to the first parameter
*
* \return nothing
*
*/
static void do_dir(const char * dir_name, const char * const * parms, const int offset)
{
	DIR *dirp;
	const struct dirent *dp;
	const char *sub_file_name;

	dirp = opendir(dir_name);

	if (dirp == NULL)
	{
		fprintf(stderr, "%s: error opening directory %s\n", *parms, dir_name);
		return;
	}

	errno = 0; //reset errno
	dp = readdir(dirp);
	
	while (dp != NULL)
	{
		sub_file_name = dp->d_name;

		//excluding . and ..
		if (strcmp(sub_file_name, ".") != 0 && strcmp(sub_file_name, "..") != 0)
		{
			char new_path[sizeof(char) * (strlen(dir_name) + strlen(sub_file_name) + 2)];

			//new dir or file path
			if (dir_name[strlen(dir_name) - 1] == '/')
			{
				sprintf(new_path, "%s%s", dir_name, sub_file_name);
			}
			else
			{
				sprintf(new_path, "%s/%s", dir_name, sub_file_name);
			}
			
			do_file(new_path, parms, offset);
		}
				
		errno = 0;
		dp = readdir(dirp);

	}
	
	if (errno != 0)
	{
		fprintf(stderr, "%s: error while processing directory %s\n", *parms, dir_name);
	}
	
	if (closedir(dirp) == -1)
	{
		fprintf(stderr, "%s: error closing directory %s\n", *parms, dir_name);
	}

	return;
}


/**
*
* \name do_print
*
* \brief The function prints the file or the file path if every filter matches
*
* \param file_name		full path of the file
* \param parms			complete parameter list
*
* \return SUCCESS if no error occured, otherwise ERROR
* \retval 0	SUCCESS
* \retval 1	ERROR
*
*/
/* ### FB The function prints the passed const char * which should contain a filename or path to stdout. It doesn't handle any filters.*/
static int do_print(const char * file_name, const char * const * parms)
{
	if (fprintf(stdout, "%s\n", file_name) < 0)
	{
		fprintf(stderr, "%s: %s - %s\n", *parms, file_name, strerror(errno));
		return ERROR;
	}

	return SUCCESS;
}


/**
*
* \name do_check_parms
*
* \brief The function checks the prompted parameters on correctness
*
* \param parms		complete parameter list
*
* \return SUCCESS if no error occured, otherwise ERROR
* \retval 0	SUCCESS
* \retval 1	ERROR
*/
static int do_check_parms(const char * const * parms)
{
	int offset = 2;
	char ** cur_Arg = (char **) (parms + offset);
		
		//runs as long as there are prompted parameters/arguments
		while (*cur_Arg != NULL)
		{	
			//compares the prompted parameters with the correct input
			if (strcmp(*cur_Arg, "-user") == 0 ||
				strcmp(*cur_Arg, "-name") == 0 ||
				strcmp(*cur_Arg, "-type") == 0 ||
				strcmp(*cur_Arg, "-path") == 0)
			{
				if (*(cur_Arg + 1) == NULL ||
					strcmp(*(cur_Arg + 1), "-user") == 0 ||
					strcmp(*(cur_Arg + 1), "-name") == 0 ||
					strcmp(*(cur_Arg + 1), "-type") == 0 ||
					strcmp(*(cur_Arg + 1), "-path") == 0 ||
					strcmp(*(cur_Arg + 1), "-nouser") == 0 ||
					strcmp(*(cur_Arg + 1), "-print") == 0 ||
					strcmp(*(cur_Arg + 1), "-ls") == 0)
				{
					/* ###FB Original error message is program : missing argument to `parms[offset]' */
					fprintf(stderr, "%s: missing additional parameters `%s'\n", *parms, *cur_Arg);
					return ERROR;
				}
				
				if (strcmp(*cur_Arg, "-user") == 0)
				{
					errno = 0; //reset errno

					signed long uid = 0;
					char * p_end;

					struct stat buf;
					
					if (lstat(*(parms + 1), &buf) == -1)
					{
						fprintf(stderr, "%s: `%s' %s\n", *parms, *(parms + 1), strerror(errno));
						return ERROR;
					}
					
					const struct passwd *pwd_entry = getpwnam(*(cur_Arg + 1));

					if (errno != 0)
					{
						/*if errno !=0 an error occured. The error is stored in errno by printf. Use strerror(errno) to return the information to the user. See above.*/
						fprintf(stderr, "%s: error while processing -user %s\n", *parms, *(cur_Arg + 1));
						return ERROR;
					}

					if (pwd_entry != NULL)
					{
						//check if user found
						if (pwd_entry->pw_uid == buf.st_uid)
						{
							return SUCCESS;
						}
					}
					else //user not found, check uid
					{
						//conversion into long int
						uid = strtol(*(cur_Arg + 1), &p_end, 10);

						if (uid == LONG_MAX || uid == LONG_MIN)
						{
							fprintf(stderr, "%s: error overflow when trying to parse -user %s\n", *parms, *(cur_Arg + 1));
							return ERROR;
						}
						if (*p_end == '\0')
						{
							//check if uid found
							if (buf.st_uid == (unsigned)uid)
							{
								return SUCCESS;
							}
						}
					}
				}
				
				if (strcmp(*cur_Arg, "-type") == 0)
				{
					
					if (strcmp(*(cur_Arg + 1), "b") && strcmp(*(cur_Arg + 1), "c") &&
						strcmp(*(cur_Arg + 1), "d") && strcmp(*(cur_Arg + 1), "p") &&
						strcmp(*(cur_Arg + 1), "f") && strcmp(*(cur_Arg + 1), "l") &&
						strcmp(*(cur_Arg + 1), "s") != 0)
					{
						/* ### FB original error message handles wrong type and too many characters in argument different. Exiting the program is correct anyways.*/
						fprintf(stderr, "%s: only one type of [bfcdpls] allowed\n", *parms);
						exit(EXIT_FAILURE);
					}
				}
				
				cur_Arg = cur_Arg + 2;
			}
			else if (strcmp(*cur_Arg, "-nouser") == 0 ||
				strcmp(*cur_Arg, "-print") == 0 ||
				strcmp(*cur_Arg, "-ls") == 0)
			{
				cur_Arg = cur_Arg + 1;
			}
			else
			{
				return ERROR;
			}
		}
			
	return SUCCESS;
}

/**
*
* \name do_user
*
* \brief The function checks if the file is owned by the prompted user
*
* The prompted username is searched and if not found, then the function 
* converts the prompted parameter to an signed long and and compares it
* with the uid. The program terminates if the parameter is not known.
*
* \param buffer		struct stat of the file
* \param parms		complete parameter list
* \param offset		offset value to the needed parameter
*
* \return SUCCESS if no error occured, otherwise ERROR or exit like Linux find
* \retval 0	SUCCESS
* \retval 1	ERROR
*
*/
static int do_user(const struct stat buffer, const char * const * parms, const int offset)
{
	errno = 0; //reset errno
	
	signed long uid = 0;
	char * p_end;
	
	const struct passwd *pwd_entry = getpwnam(*(parms + offset));

	if (errno != 0)
	{
		/* ### FB same here. Errno is set to give information on what went wrong. Use strerror(errno)*/
		fprintf(stderr, "%s: error while processing -user %s\n", *parms, *(parms + offset));
		return ERROR;
	}

	if (pwd_entry != NULL)
	{
		//check if user found
		if (pwd_entry->pw_uid == buffer.st_uid)
		{
			return SUCCESS;
		}
	}
	else //user not found, check uid
	{
		//conversion into long int
		uid = strtol(*(parms + offset), &p_end, 10);

		if (uid == LONG_MAX || uid == LONG_MIN)
		{
			fprintf(stderr, "%s: error overflow when trying to parse -user %s\n", *parms, *(parms + offset));
			return ERROR;
		}
		if (*p_end == '\0')
		{
			//check if uid found
			if (buffer.st_uid == (unsigned) uid)
			{
				return SUCCESS;
			}
		}
		else
		{
			fprintf(stderr, "%s: %s is not the name of a known user\n", *parms, *(parms + offset));
			exit(EXIT_FAILURE);
		}
		
	}
		
	return ERROR;
}


/**
*
* \name do_name
*
* \brief The function checks and compares the filename with the prompted parameter
*
* \param file_name		path to the file
* \param parms			full parameter list
* \param offset			offset value to the needed parameter
*
* \return SUCCESS if parameter is matching, otherwise ERROR
* \retval 0	SUCCESS
* \retval 1	ERROR
*
*/
static int do_name(const char * file_name, const char * const * parms, const int offset)
{
	int fnmatch_ret;
	/* ### FB comment is unclear. Does this mean variable length array?*/
	char temp_file_name[strlen(file_name) + 1]; //VLA
	char *base_name;

	strcpy(temp_file_name, file_name);
	base_name = basename(temp_file_name);

	//check if basename matches prompted parameter
	fnmatch_ret = fnmatch(*(parms + offset), base_name, FNM_NOESCAPE);
	
	if (fnmatch_ret == 0)
	{
		return SUCCESS;
	}
	else if (fnmatch_ret != FNM_NOMATCH)
	{
		fprintf(stderr, "%s: error matching name to %s\n", *parms, *(parms + offset));
	}

	return ERROR;
}


/**
*
* \name do_nouser
*
* \brief The function checks if there is a file unowned
*
* \param buffer		struct stat of the file
* \param parms		complete parameter list

* \return SUCCESS if no error occured, otherwise ERROR or exit like Linux find
* \retval 0	SUCCESS
* \retval 1	ERROR
*
*/
static int do_nouser(const struct stat buffer, const char * const * parms)
{	
	errno = 0; //reset errno

	const struct passwd *pwd_entry = getpwuid(buffer.st_uid);
	
	//check if file is with nouser, else user or errno exit like find
	if ((pwd_entry == NULL) && (errno == 0))
	{
		return SUCCESS;
	}
	else if (errno != 0)
	{
		/* ### FB If errno is not Zero this doesn't mean non-owned files don't exist. manpage : 
		*       "The getpwnam() and getpwuid() functions return a pointer to a passwd
		*		structure, or NULL if the matching entry is not found or an error
		*		occurs.  If an error occurs, errno is set appropriately."
		* 		no file without user would be (pwd_entry != NULL && errno == 0)
		* 		additionally the error message is not providing information stored in errno.
		*/
		fprintf(stderr, "%s: no file without user \n", *parms);
		exit(EXIT_FAILURE);
	}
	
	return ERROR;
}

/* ### FB output like ls is not very informative since ls could also mean the commandline ls. Suggestion: prints output in ls -disl format would be better.*/
/**
*
* \name do_ls
* 
* \brief The function makes output like ls with all useful data of the files
*
* \param buffer			struct stat of the file
* \param file_name		path to the file
* \param parms			complete parameter list

* \return SUCCESS if no error occured, otherwise ERROR
* \retval 0	SUCCESS
* \retval 1	ERROR
*/
static int do_ls(const struct stat buffer, const char * file_name, const char * const * parms)
{
	struct passwd* user;
	struct group* group;
	struct tm* time;
	/* ### FB to be able to determine how long char mode[] actually is you could also write a makro determining the length of the array */
	char mode[] = { "----------" };
	/* ### FB month is receiving 3 characters max if using abbreviated so month would need only 4 bytes instead of 10 */
	char month[10];
	char* user_name = "\0";
	/* ### FB please leave a comment why 13 is the correct length or size of the array, or use a MACRO and comment it there. */
	char uid[13];
	char* group_name = "\0";
	/* ### FB please leave a comment why 13 is the correct length or size of the array, or use a MACRO and comment it there. */
	char gid[13];
	/* ### FB please leave a comment why 13 is the correct length or size of the array, or use a MACRO and comment it there. */
	char time_disp[13] = { 0 }; //for 0 termination
	
	unsigned long blocks = 0l;

	errno = 0;
	/* ### FB helper function get_file_type() exists
	*	mode[0] = (get_file_type(buffer.st_mode) != 'f') ? get_file_type(buffer.st_mode) : '-' ;
	* 	for instance could be used.
	*/	
	switch (buffer.st_mode & S_IFMT)
	{
		case S_IFREG:	//regular file
			mode[0] = '-';	break;
		case S_IFDIR:	//directory
			mode[0] = 'd';	break;
		case S_IFCHR: 	//character device 
			mode[0] = 'c';	break;
		case S_IFBLK:	//block device 
			mode[0] = 'b';	break;
		case S_IFIFO:	//FIFO
			mode[0] = 'f';	break;
		case S_IFLNK:	//symbolic link 
			mode[0] = 'l';	break;
		case S_IFSOCK:	//socket
			mode[0] = 's'; 	break;
		default:		//default
			mode[0] = '?'; 	break;
	}

	if (S_IRUSR & buffer.st_mode) 
		mode[1] = 'r';
	if (S_IWUSR & buffer.st_mode)
		mode[2] = 'w';
	
	if ((S_IXUSR & buffer.st_mode) && !(S_ISUID & buffer.st_mode)) 
		mode[3] = 'x';
	else if (S_IXUSR & buffer.st_mode)
		mode[3] = 's';
	else if (S_ISUID & buffer.st_mode) 
		mode[3] = 'S';

	if (S_IRGRP & buffer.st_mode)
		mode[4] = 'r';
	if (S_IWGRP & buffer.st_mode)
		mode[5] = 'w';
	
	if ((S_IXGRP & buffer.st_mode) && !(S_ISGID & buffer.st_mode))
		mode[6] = 'x';
	else if (S_IXGRP & buffer.st_mode)
		mode[6] = 's';
	else if (S_ISGID & buffer.st_mode)
		mode[6] = 'S';


	if (S_IROTH & buffer.st_mode)
		mode[7] = 'r';
	if (S_IWOTH & buffer.st_mode)
		mode[8] = 'w';
	
	if ((S_IXOTH & buffer.st_mode) && !(S_ISVTX & buffer.st_mode))
		mode[9] = 'x';
	else if (S_IXOTH & buffer.st_mode)
		mode[9] = 't';
	else if (S_ISVTX & buffer.st_mode)
		mode[9] = 'T';

	//blocks 
	if (mode[0] != 'l')
	{
		blocks = (unsigned long)buffer.st_blocks;
		
		if (getenv("POSIXLY_CORRECT") == NULL)
		{
			blocks = ((unsigned long)buffer.st_blocks / 2 + buffer.st_blocks % 2);

		}
	}

	//user
	if ((user = getpwuid(buffer.st_uid)) == NULL || user->pw_name == NULL)
	{
		if (errno != 0)
		{
			fprintf(stderr, "%s: %s - %s\n", *parms, file_name, strerror(errno));
		}
		else
		{
			/* ### FB No error handling: What if an encoding error occurs?*/
			snprintf(uid, sizeof(uid), "%d", buffer.st_uid);
			user_name = uid;
		}

	}
	else
	{
		user_name = user->pw_name;
	}

	errno = 0;

	//group 
	if ((group = getgrgid(buffer.st_gid)) == NULL || (group->gr_name == NULL))
	{
		if (errno != 0)
		{
			fprintf(stderr, "%s: %s - %s\n", *parms, file_name, strerror(errno));
		}
		else
		{
			/* ### FB No error handling: What if an encoding error occurs?*/
			snprintf(gid, sizeof(gid), "%d", buffer.st_gid);
			group_name = gid;
		}
	}
	else
	{
		group_name = group->gr_name;
	}
	
	errno = 0;
	
	time = localtime(&(buffer.st_mtime));
	strftime(month, sizeof(month), "%b", time);

	if (sprintf(time_disp, "%s %2d %02d:%02d", month, time->tm_mday, time->tm_hour, time->tm_min) < 0)
	{
		fprintf(stderr, "%s: unable to create string", *parms);
	}
	

	char file_string[strlen(file_name)];

	if (mode[0] != 'l')
	{
		strcpy(file_string, file_name);
	}
	else if (mode[0] == 'l')
	{
		char linkbuf[buffer.st_size + 1]; //VLA

		const int charsread = readlink(file_name, linkbuf, buffer.st_size);
		
		if (charsread == -1)
		{
			fprintf(stderr, "%s: error to read link: %s", *parms, strerror(errno));
			return ERROR;
		}
		else
		{
			linkbuf[buffer.st_size] = '\0';
		}

	}
	/* ### FB errno not set*/
	if (fprintf(stdout, "%6ld %4ld %s %3d %s %s %8ld %s %s\n", buffer.st_ino, blocks, mode, buffer.st_nlink, user_name, group_name, buffer.st_size, time_disp, file_string) < 0)
	{	
		/* ### FB if fprintf fails errno is set so an error message could also be obtained by using strerror(errno)*/
		fprintf(stderr, "%s:  error %s", *parms, file_name);
		return ERROR;
	}

	return SUCCESS;
}


/**
*
* \brief The function checks if the prompted type matches the file type
*
* \param buffer		struct stat of the file
* \param parms		complete parameter list
* \param offset		offset value to the needed parameter (bfcdpls)
*
* \return SUCCESS if type is matching, otherwise ERROR
* \retval 0 not matching
* \retval 1 matching
*
*/
static int do_type(const struct stat buffer, const char * const* parms, const int offset)
{
	char type_char = '-';

	//check type char of file_name
	type_char = get_file_type(buffer.st_mode);
			
	if (type_char == **(parms + offset))
	{	
		return SUCCESS;
				
	}
	
	return ERROR;
}

/* ###FB if handed in a second parameter the function could return the right character for ls right away.*/
/**
*
* \brief The function returns the type char (bfcdpls) of the given mode of the file
*
* \param mode	 mode of file
*
* \return type corresponding char or default '-'
*
*/
static char get_file_type(const mode_t mode)
{
	char type = '-';
	
	if (S_ISBLK(mode))
	{
		type = 'b';
	}
	else if (S_ISREG(mode))
	{
		type = 'f';
	}
	else if (S_ISCHR(mode))
	{
		type = 'c';
	}
	else if (S_ISDIR(mode))
	{
		type = 'd';
	}
	else if (S_ISFIFO(mode))
	{
		type = 'p';
	}
	else if (S_ISLNK(mode))
	{
		type = 'l';
	}
	else if (S_ISSOCK(mode))
	{
		type = 's';
	}
	return type;
}


/**
*
* \name do_path
*
* \brief The function checks the path pattern against the prompted filepath
*
* \param file_name		path of the file
* \param parms			complete parameter list
* \param offset			offset value to the needed parameter

* \return SUCCESS if no error occured, otherwise ERROR
* \retval 0	SUCCESS
* \retval 1	ERROR
*
*/
static int do_path(const char * file_name, const char * const * parms, const int offset)
{
	int fnmatch_result;

	fnmatch_result = fnmatch(*(parms + offset), file_name, FNM_NOESCAPE);

	if (fnmatch_result == 0)
	{
		return SUCCESS;
	}

	return ERROR;
}

// =================================================================== eof ==

// Local Variables:
// mode: c
// c-mode: k&r
// c-basic-offset: 8
// indent-tabs-mode: t
// End: