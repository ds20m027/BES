//*
/*!\file myfind.c
*
* Betriebssysteme myfind-program File.
* Beispiel 1
*
* \author
* - Magdalena Andrae <ic17b079@technikum-wien.at>
* - Rainhardt Gabriel <ic17b078@technikum-wien.at>
* \date 2018/03/01
*
* \version 001
*
* \todo Review it for missing error checks.
* \todo link appent to path
* \todo fix formatting of ls
* \todo test more completely
*
*
* Last modified 2018/04/07.
* Last modified by Rainhardt
*/
// -------------------------------------------------------------- includes --
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fnmatch.h>
#include <libgen.h>
#include <limits.h>

// --------------------------------------------------------------- defines --
/*! \brief TRUE is a MAKRO used for boolean logic */
#define TRUE 1
/*! \brief FALSE is a MAKRO used for boolean logic */
#define FALSE 0
/*! \brief MISSING_ARG is a check value for validate_options() if an argument to an option is missing */
#define MISSING_ARG 2
/*! \brief FILE_INFO_STRING is a MAKRO determining the string length for the ownership and file information of files or directories */
#define FILE_INFO_STRING 10
/*! \brief MAX_OPTION_STRING_LENGTH is a MAKRO determining the maximum string length of an option string stored in struct opt_struct */
#define MAX_OPTION_STRING_LENGTH 6
/*! \brief TYPE_OPTIONS_CNT is a MAKRO that represents the count of valid options */
#define TYPE_OPTIONS_CNT 7
/*! \brief MAX_UID_GUID_STR_LENGTH represents the maximum digit_length an integer 2^32-1 can have in base 10. '\0' */
#define MAX_UID_GUID_STR_LENGTH 11 
/*! \brief BLOC_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define BLOC_TYPE 100
/*! \brief CHAR_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define CHAR_TYPE 101
/*! \brief DIR_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define DIR_TYPE  102
/*! \brief PIPE_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define PIPE_TYPE 103
/*! \brief FILE_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define FILE_TYPE 104
/*! \brief LINK_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define LINK_TYPE 105
/*! \brief SOCK_TYPE is a MAKRO to be able to use switch case for types of files or directories */
#define SOCK_TYPE 106
/*! \brief NAME_OPT is a MAKRO to be able to use switch case for options and to return the string of an option if needed of files or directories */
#define NAME_OPT  1001
/*! \brief TYPE_OPT is a MAKRO to be able to use switch case for options and to return the string of an option if needed of files or directories */
#define TYPE_OPT  1002
/*! \brief USER_OPT is a MAKRO to be able to use switch case for options and to return the string of an option if needed of files or directories */
#define USER_OPT  1003
/*! \brief LS_OPT is a MAKRO to be able to use switch case for options and to return the string of an option if needed of files or directories */
#define LS_OPT    1004
/*! \brief PRINT_OPT is a MAKRO to be able to use switch case for options and to return the string of an option if needed of files or directories */
#define PRINT_OPT 1005


// -------------------------------------------------------------- typedefs --
/*! \brief opt_struct is used for storing the option information: the option itself, the number of expected arguments for verification and a mask value
 */
struct opt_struct {
    char option [MAX_OPTION_STRING_LENGTH+1];
    int opt_args;
    int mask_val;
};
/*! \brief type_struct is used for storing chars of valid types and a mask value. This struct is used for the "-type" function.
 */
struct type_struct {
    char type[2];
    int type_val;
};
// --------------------------------------------------------------- globals --
/*! \brief The program is set to argv[0] in order for print_error() to access it without getting it as a parameter
*/
static const char *program = "<not yet set>";
/*! \brief param_cnt saves the argc and is used by practically all functions
 */
static int param_cnt = 0;
/*! \brief valid_options contains all implemented action-strings, the number of expected arguments after each string, and a mask the option can be identified by
 */
static struct opt_struct valid_options[]= {
    { .option = "-print", .opt_args = 0,.mask_val = PRINT_OPT},
    { .option = "-ls"	, .opt_args = 0,.mask_val = LS_OPT},
    { .option = "-type"	, .opt_args = 1,.mask_val = TYPE_OPT},
    { .option = "-name"	, .opt_args = 1,.mask_val = NAME_OPT},
    { .option = "-user"	, .opt_args = 1,.mask_val = USER_OPT}
};
/*! \brief type_chars contains all valid types [bcdpfls] of a file and it is used in the function compare_type()
 */
static struct type_struct type_chars[] =
{
    { .type = "b", .type_val = BLOC_TYPE},
    { .type = "c", .type_val = CHAR_TYPE},
    { .type = "d", .type_val = DIR_TYPE	},
    { .type = "p", .type_val = PIPE_TYPE},
    { .type = "f", .type_val = FILE_TYPE},
    { .type = "l", .type_val = LINK_TYPE},
    { .type = "s", .type_val = SOCK_TYPE}
};
// ------------------------------------------------------------- functions --
static void do_file(const char *file_name, const char * const *parms); 
static void do_dir(const char * dir_name, const char * const *parms);
static int action_print(const char *file_name);
static int action_ls(const char *file_name, const struct stat *file_info);
static int print_error(const char *message, const char *file_name,const int my_errno);
static void print_usage(const char *message);
static int validate_options(const char * const *parms);
static int compare_name(const char *file_name, const char *compstring);
static int compare_user(const char *file_name,struct stat *file_info, const char *compstring);
static int compare_type(const char *file_name,struct stat *file_info, const char *compstring);
static int ret_mask_to_opt_string(const char *arg);
static int ret_arg_exp(const int mask);
static char *ret_opt_on_mask(const int mask);
static long long convert_str_to_uid(const char *string);
static char *combine_paths(const char* src_1,const char * src_2);
/*! \brief A smaller find program
*
* This is the main entry point for any C program.
*
* \param argc the number of arguments typed into the command line.
* \param argv the command line input/the arguments themselves. The program name is held in argv[0].
*
* \retval EXIT_SUCCESS
* \retval EXIT_FAILURE
*/
int main(int argc,
         const char *argv[])
{
    param_cnt = argc;
    int _validate_val = FALSE;
    errno = 0;
    //check whether there was any (valid) input, if not "error", otherwise programm goes on
    if(argv[0]==NULL) {
        print_error("Error in Main","program name was not passed",errno);
        exit (EXIT_FAILURE);
    }
    program = argv [0];
    //check if we got any options/arguments for our program. If there were less than 2 arguments, the programm is missing vital information and stops at this point
    if(param_cnt < 2) {
        print_usage("no path passed");
        return EXIT_FAILURE;
    }
    _validate_val = validate_options(argv);
    if(_validate_val==FALSE) {
        print_usage("option doesn't exist");
        return EXIT_FAILURE;
    }
    if(_validate_val != TRUE) {
        if(fprintf(stderr,"%s: missing argument to `%s'\n",program,ret_opt_on_mask(_validate_val)) < 0) {
            print_error("an error occured when printing to stderr\n","",errno);
            return EXIT_FAILURE;
        } else return EXIT_FAILURE;
    }

    do_file(argv[1],argv);
    //no errors occured - quit the program
    return EXIT_SUCCESS;
}
/*! \brief prints errors with a message
*
* This function is called if an error occured. If the error number (errno != 0) is activated  an additional error message is provided by strerror(errno)
* The output is "stderr" and contains the name of the program, the message and the file.
*
* \param message string that contains the message in which function something went wrong
* \param file_name string that contains the file or directory entry where myfind produced an error
* \param my_errno saved errno value or 0
*
* \retval EXIT_SUCCESS
* \retval EXIT_FAILURE
*/
static int print_error(const char *message, const char * file_name,const int my_errno) {
    if (my_errno!=0) {
        fprintf(stderr,"%s :%s(\"%s\") failed: %s\n",program,message,file_name,strerror(my_errno));
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
/*! \brief do_file() gathers information about a file and passes its name to action_print()
*	if the file opened is a directory it calls do_dir() and passes the directories name
*
* \param file_name is a string containing a directoryname or a filename
* \param parms are all the arguments given to the command line including the program name itself at index 0
*
* \retval none
*/
static void do_file(const char * file_name, const char * const *parms) {
    errno = 0;
    int _parms_index = 2;
    int _printing_flag = FALSE; //default: we want to print if only the path is given
    int _handle_printing_flag = TRUE; //default: we want to print - if not it will be set by the comparisons type/name/user
    int _next_opt = 0;
	int _check = FALSE;
	int _store_options [param_cnt-1];//-1 for program name -1 for path +1 for terminating '\0'
	if(param_cnt ==2){_store_options[0]=PRINT_OPT;}; // default action if no options are passed
    struct stat _file_info;
    // errorhandling
    if(lstat (file_name,&_file_info)==-1) {
        print_error("lstat :",file_name,errno);
    };
    while ((parms[_parms_index]!=NULL)) {
        //process all option parameters and store them in an array with their truth value so we can examine the array later on to see if we should print anything at all
        _next_opt = 0;
        if((strcmp(parms[_parms_index],"-name") == 0)) {
            _printing_flag = (parms[_parms_index+2]==NULL)?FALSE:TRUE;
			_check = compare_name(file_name,parms[_parms_index+1]);
            _handle_printing_flag = _check?(parms[_parms_index+2]!=NULL)? TRUE : !_printing_flag : FALSE ;
			_store_options[_parms_index-2]= ret_mask_to_opt_string(parms[_parms_index]);
			_store_options[_parms_index-1]= _check;
            _next_opt = 1;
        } else if((strcmp(parms[_parms_index],"-user"))== 0) {
            _printing_flag = (parms[_parms_index+2]==NULL)?FALSE:TRUE;
			_check = compare_user(file_name,&_file_info,parms[_parms_index + 1]);
            _handle_printing_flag = _check?(parms[_parms_index+2]!=NULL)? TRUE : !_printing_flag : FALSE ;
			_store_options[_parms_index-2]= ret_mask_to_opt_string(parms[_parms_index]);
			_store_options[_parms_index-1]= _check;
            _next_opt = 1;
        } else if (((strcmp(parms[_parms_index],"-type")) == 0)) {
            _printing_flag = (parms[_parms_index+2]==NULL)?FALSE:TRUE;
			_check = compare_type(file_name,&_file_info,parms[_parms_index +1]);
            _handle_printing_flag = _check?(parms[_parms_index+2]!=NULL)? TRUE : !_printing_flag : FALSE ;
			_store_options[_parms_index-2]= ret_mask_to_opt_string(parms[_parms_index]);
			_store_options[_parms_index-1]= _check;
            _next_opt = 1;
        } else if(strcmp(parms[_parms_index],"-print") == 0 && _handle_printing_flag == TRUE) {
			_store_options[_parms_index-2]= ret_mask_to_opt_string(parms[_parms_index]);
            //action_print(file_name);
            _printing_flag = TRUE;
        } else if((strcmp(parms[_parms_index],"-ls") == 0) && (_handle_printing_flag == TRUE)) {
			_store_options[_parms_index-2]= ret_mask_to_opt_string(parms[_parms_index]);
            //action_ls(file_name,&_file_info);
            _printing_flag = TRUE;
        };
        _parms_index ++;
        _parms_index += _next_opt;
    }
	if(param_cnt == 2){
		action_print(file_name);
	}else{
		_check = TRUE; // we asume that we got a valid input otherwise _check will change to FALSE
		for(int _index = param_cnt-3; _index != 0; _index--){
			_check = ((_store_options[_index]!= FALSE)&&_check)? TRUE : FALSE;
		}
		if(_check){
			for(int _index = 0; _store_options[_index]!= 0; _index ++){
				switch(_store_options[_index]){
					case PRINT_OPT : 
						action_print(file_name);
						_printing_flag = TRUE;
						break;
					case LS_OPT : action_ls(file_name,&_file_info);
						_printing_flag = TRUE;
						break;
					default : /*do nothing*/;
						break;
				}
			}
			if(_printing_flag == FALSE){ action_print(file_name);}
		}
	}
    /* check if it is a directory */
    if(S_ISDIR(_file_info.st_mode)) {
        do_dir(file_name,parms);
    }
}
/*! \brief opens, reads from and closes directories
*
* This function is called by the function do_file() and opens, reads and closes the directories.
* It parses the directory's entries to a string and hands it back to do_file()
*
* \param dir_name is the directory string
* \param parms are the program's parameters
*
* \retval none
*/
static void do_dir(const char * dir_name, const char * const * parms) {
    errno = 0;
    DIR *_path = NULL;
    struct dirent *_dir_ent = NULL;
    _path = opendir(dir_name); //Error check missing
    if(!(_path = opendir(dir_name))&& errno != 0) {
        print_error("opendir : Error opening directory",dir_name,errno);
		return;
    };
	errno = 0;
	while((_dir_ent = readdir(_path))){

        if ((strcmp(_dir_ent->d_name,".") != FALSE ) && (strcmp(_dir_ent->d_name,"..") != FALSE)) {
			char *_path_combined;
			/*print the combined path to _path_combined*/
			_path_combined = combine_paths(dir_name,_dir_ent->d_name);
			if(_path_combined == NULL){
				print_error("error in combine_paths",dir_name,0);
				free(_path_combined);
				exit (EXIT_FAILURE);
			}

			/* takes the directory stream so we can continue after closing the directory */
			errno = 0;
			long _tell_dir_loc = telldir(_path); /*get the file descriptor*/
			if(errno != 0 && _tell_dir_loc < 0){
				print_error("error in telldir()",dir_name,errno);
				free (_path_combined);
				exit (EXIT_FAILURE);
			}

            /* and now close the directory */
            errno = 0;
            if(closedir(_path)< 0 && errno != 0){
				print_error("error in closedir()",dir_name,errno);
				free(_path_combined);
				exit (EXIT_FAILURE);
			}
			do_file(_path_combined,parms);
			free (_path_combined);
			errno =0;
			if(!(_path = opendir(dir_name))&& errno != 0){
				print_error("error in opendir()",dir_name,errno);
				return;
			}
			seekdir(_path,_tell_dir_loc); 
            /*no error handling needed. seekdir returns nothing*/
		}
	}
	if(_dir_ent == NULL && errno != 0) {
            print_error ("readdir : Error reading from directory",dir_name,errno);
			exit(EXIT_FAILURE);
    };
	errno = 0;
	/*close directory again before we leave function*/
	if(closedir(_path) < 0 && errno != 0){
		print_error("error in closedir()",dir_name,errno);
	}
	return;
}
/*! \brief combine_paths
 * 	combines two pathstrings to one pathstring and returns the newly created pathstring or NULL if an Error occured
 * 	\param src_1 string of first path
 * 	\param src_2 string of to ammend path
 * 
 * 	\retval combined_path if everything was successful
 * 	\retval NULL if failure occured
 */
static char * combine_paths(const char* src_1,const char * src_2){
	if(src_1 == NULL) return NULL;
	char *_combined_path =(src_2 == NULL)? ((char *) malloc(((strlen(src_1)+2)*sizeof(char)))) : ((char *) malloc((strlen(src_1)+strlen(src_2)+2)*sizeof(char)));
	if(_combined_path == NULL){
		print_error("error allocating memory","Unknown",0);
		exit(EXIT_FAILURE);
	}
	if(sprintf(_combined_path,"%s%s%s",src_1,src_1[strlen(src_1)-1]=='/' ? "" : "/" , (src_2 == NULL)? "\0" : src_2 )<0){
		return NULL;
	}
	
	return _combined_path;
}
/*! \brief action_print() prints the passed string (a file_name or a directory_name) to stdout 
*   It is called either when "-print" is the only option passed to the programm or
*   the action-input following the path name is valid and the output need to be printed
* 	if printing fails it calls the function print_error()
*
* \param file_name is the filename to be printed
*
* \retval none
*/
static int action_print(const char* file_name) {

    if(fprintf(stdout,"%s\n",file_name)<0) {
        print_error("fprintf : Error writing to stdout\n",file_name,errno);
        exit (EXIT_FAILURE);
    };
    return EXIT_SUCCESS;

}
/*! \brief
*	print the file or directory name as well as the following details to stdout
*   - inode number
*   - number of blocks
*   - permissions
*   - number of links
*   - owner
*   - group
*   - last modification time
*   - path
*
* \param file_name
* \param file_info
*
* \retval none
*/
static int action_ls(const char* file_name,const struct stat *file_info) {
    /* inode number */
    int _my_errno = 0;
    long unsigned _inode_nr = file_info->st_ino;
    /* number of blocks - counts if it is not a link */
    long unsigned _block_count = file_info->st_blocks;
    if ((file_info->st_mode & S_IFMT)==S_IFLNK) {};
    /* no error can occur with getenv() */
    char *posixly=getenv("POSIXLY_CORRECT");
    if(posixly==NULL) {
        _block_count = ((file_info->st_blocks)+1)/2;
    };
    // information about the file "-" is a file / "d" is a directory / "l" is a link
    char _file_type [2] ="";
    char _permissions_field_own [4] ="";
    char _permissions_field_grp [4] ="";
    char _permissions_field_oth [4] ="";
    for (int _index= 0; _index < 3; _index++) {
        if(_index == 0) {
            //type of file
            switch(file_info->st_mode & S_IFMT) {
            case S_IFDIR:
                _file_type[_index] 	= 'd';
                break;
            case S_IFIFO:
                _file_type[_index] 	= 'p';
                break;
            case S_IFLNK:
                _file_type[_index] 	= 'l';
                break;
            case S_IFREG:
                _file_type[_index]	= '-';
                break;
            case S_IFCHR:
                _file_type[_index]	= 'c';
                break;
            case S_IFBLK:
                _file_type[_index]	= 'b';
                break;
            case S_IFSOCK:
                _file_type[_index]	= 's';
                break;
            default:
                print_error("-ls Function",file_name,errno) ;
                break;
            };
            // read permissions
            _permissions_field_own[_index] = (file_info->st_mode & S_IRUSR)?'r':'-' ;
            _permissions_field_grp[_index] = (file_info->st_mode & S_IRGRP)?'r':'-' ;
            _permissions_field_oth[_index] = (file_info->st_mode & S_IROTH)?'r':'-' ;
        };
        //write permissions
        if (_index == 1) {
            _permissions_field_own[_index] = (file_info->st_mode & S_IWUSR)?'w':'-' ;
            _permissions_field_grp[_index] = (file_info->st_mode & S_IWGRP)?'w':'-' ;
            _permissions_field_oth[_index] = (file_info->st_mode & S_IWOTH)?'w':'-' ;
        };
        //execute permissions
        if (_index == 2) {
            _permissions_field_own[_index] = ((file_info->st_mode & S_IXUSR)?
                                              ((file_info->st_mode & S_ISVTX)?'S':'x'):((file_info->st_mode & S_ISVTX)?'s':'-'));
            _permissions_field_grp[_index] = ((file_info->st_mode & S_IXGRP)?
                                              ((file_info->st_mode & S_ISVTX)?'S':'x'):((file_info->st_mode & S_ISVTX)?'s':'-'));
            _permissions_field_oth[_index] = ((file_info->st_mode & S_IXOTH)?
                                              ((file_info->st_mode & S_ISVTX)?'T':'x'):((file_info->st_mode & S_ISVTX)?'t':'-'));
        };
    };

    // user that owns the file
    char _user_id[MAX_UID_GUID_STR_LENGTH];
    errno = _my_errno;
    struct passwd *_p_user = getpwuid(file_info->st_uid);
    if(_p_user==NULL){
        if(errno!=0){
            print_error("getpwuid : Error receiving user",file_name,errno);
            exit (EXIT_FAILURE);
        }
        if(sprintf(_user_id, "%u", file_info->st_uid)<0){
            exit (EXIT_FAILURE);
        }
    };
    // group that owns the file
    char _group_id[MAX_UID_GUID_STR_LENGTH];
    errno = _my_errno;
    struct group *_p_group = getgrgid(file_info->st_gid);
    if(_p_group==NULL){
        if (errno!=0){
            print_error("getgrgid : Error receiving group",file_name,errno);
            exit (EXIT_FAILURE);
        }
        else {
            if(sprintf(_group_id, "%u", file_info->st_gid)<0){
            exit (EXIT_FAILURE);
        }
    }
    };
    // size of file or directory
	char _file_size [9];
	if((file_info->st_size == 0) && ((S_ISCHR(file_info->st_mode)) || (S_ISBLK(file_info->st_mode)))){
		_file_size[0] = '\0';
	}else{
		sprintf(_file_size,"%ld",file_info->st_size);
	}
    // date of last modification;
    char _date [13];
    struct tm *_timeinfo;
    _timeinfo = localtime(&(file_info->st_mtime));
    if(strftime(_date,13,"%b %e %H:%M",_timeinfo) == 0) {
        print_error("Error in strftime",file_name,errno);
    };
    // file name
    char *_p_link_string = NULL;
    char *_buffer = NULL;
    char _arrow[5] = " -> ";
    int _buf_cnt = 0;
    if(S_ISLNK(file_info->st_mode)) {
        _p_link_string=(char*)malloc((file_info->st_size)+strlen(_arrow)+1);
        int _my_errno = 0;
        _my_errno = errno;
        if(_p_link_string == NULL) {
            print_error("error allocating memory",file_name,_my_errno);
            exit (EXIT_FAILURE);
        };
        strcpy(_p_link_string,_arrow);
        _my_errno = 0;
        _buf_cnt = readlink(file_name,&_p_link_string[strlen(_arrow)],file_info->st_size);
        if(_buf_cnt == -1) {
            print_error("readlink : Error reading link",file_name,errno);
            free(_buffer);
            free(_p_link_string);
            exit (EXIT_FAILURE);
        };
        if(_buf_cnt+(strlen(_arrow)) == sizeof(_p_link_string)) {
            _buffer = realloc(_p_link_string,(file_info->st_size+1));
            if( _buffer != NULL) {
                _p_link_string = _buffer;
            } else {
                print_error("error reallocating memory",file_name,errno);
                free(_buffer);
                free(_p_link_string);
                exit (EXIT_FAILURE);
            };
        };
        _p_link_string[file_info->st_size+strlen(_arrow)]= '\0';
    };
    if(fprintf(stdout,"%9lu %4lu %.1s%.3s%.3s%.3s %4u %s %s \t%-s \t%.12s %s%s\n",

		_inode_nr,
		_block_count,
		_file_type,
		_permissions_field_own,
		_permissions_field_grp,
		_permissions_field_oth,
		file_info->st_nlink,
        _p_user != NULL? _p_user->pw_name : _user_id,
		_p_group != NULL?_p_group->gr_name : _group_id,
		_file_size,
		_date,
		file_name,
		S_ISLNK(file_info->st_mode)? &_p_link_string[0]: ""
	) < 0) {
        print_error("fprintf : Error writing to stdout\n",file_name,errno);
        exit (EXIT_FAILURE);
    };
    free (_p_link_string);
    free (_buffer);
    return EXIT_SUCCESS;
}

/*! \brief print_usage
*	prints information on how to use the program to stdout
*   if printing fails it calls the function print_error()
*  \param message 
*
* 	\retval none
*/

static void print_usage(const char *message) {
    char _usage_msg [] = "Usage: myfind <directory> <test-aktion>...\n\t-user <name/uid>\n\t-name <glob-pattern>\n\t-type [bcdpfls]\n\t-print\n\t-ls\n";
    if(fprintf(stdout,"%s:\n%s:%s \n",message,program,_usage_msg)<0) {
        print_error("Error in fprintf","print_usage()\n",errno);
    };
}
/*! \brief validate_options This function examines the given command line arguments for existing options.
 * 	in case the pattern is missing or wrong it prints information about expected arguments and quits the program
 *
 * 	\param parms argument vector starting from the 3rd argument
 *
 * \retval _arg_expected option masks if options are valid PRINT_OPT, LS_OPT, TYPE_OPT, NAME_OPT, USER_OPT
 * \retval FALSE if options are not valid
 *
 */
static int validate_options(const char * const *parms) {
    int _is_valid = FALSE;
    int _opt_expected = FALSE;
    int _opt_array[param_cnt-1];// first param and second param will not be checked 
    int _opt_marker = 0;
    for(int i =0; i<param_cnt; i++) {
        _opt_array[i]=0;
    }
    int _err_flag = FALSE;
    //first save valid arguments into the array
    for(int _parms_index = 2,_opt_ar_in = 0; _parms_index < param_cnt; _parms_index++,_opt_ar_in++) {
        _is_valid = ret_mask_to_opt_string(parms[_parms_index]);
        _is_valid = ret_mask_to_opt_string(parms[_parms_index]);
        _opt_array[_opt_ar_in] = _is_valid;
        _opt_expected = (!ret_arg_exp(_opt_array[_opt_ar_in])&&parms[_parms_index+1]==NULL);
        if(!_opt_expected&&(_parms_index+1!=param_cnt)) {
            _opt_array[_opt_ar_in+1]=1;
        }else if((!_opt_expected)&&(_parms_index+1==param_cnt)){
			// first parameter misses arg
			return _opt_array[_opt_ar_in];
		}
        if(_opt_array[_opt_ar_in] == 0){
			if(fprintf(stderr,"%s:paths must precede expression: %s",program,parms[_parms_index])){
				print_error ("error in fprintf()",program,errno);
				exit(EXIT_FAILURE);
			}
			exit (EXIT_FAILURE);
		}
        _parms_index += ret_arg_exp(_is_valid);
        _opt_ar_in += ret_arg_exp(_is_valid);
    }
    //now validate the array
    _opt_expected = FALSE; //reset
    for(int i = 0; (i <param_cnt-1)&&!_err_flag; i++) {
        switch(_opt_array[i]) {
        case PRINT_OPT :
            _opt_expected = TRUE;
            break;
        case LS_OPT :
            _opt_expected = TRUE;
            break;
        case TYPE_OPT : {
            _opt_expected = (_opt_array[i+1]==1)?TRUE:FALSE;
        };
        break;
        case NAME_OPT :
            _opt_expected = (_opt_array[i+1]==1)?TRUE:FALSE;
            break;
        case USER_OPT :
            _opt_expected = ((_opt_array[i+1]==1))?TRUE:FALSE;
            break;
		default : ;
			break;
        }
        //check if opt_expected is true  - if not the program handles it as a wrong input
        if(!_opt_expected) {
            _err_flag = ((i==0)||(_err_flag == MISSING_ARG))?  MISSING_ARG : (_err_flag == FALSE)? MISSING_ARG : TRUE;
            _opt_marker = (_err_flag == MISSING_ARG) ? (_opt_array[i]): 0 ;
        };
        // if nothing was returned we can go on and set i to the appropriate number
        i = (_opt_expected&&!(_opt_array[i]==(LS_OPT|PRINT_OPT))) ? i+1 : i ;

    }
    if(param_cnt==2){_err_flag =FALSE;};//it is a path
    if(_err_flag !=FALSE) {
        return (_err_flag == MISSING_ARG) ? _opt_marker : FALSE ;
    } else {
        return TRUE;
    };

}
/*! \brief ret_arg_exp is called from validate_options() and checks if argument is expected by the option
 *
 * \retval TRUE if an argument is expected by an option
 * \retval FALSE if no argument is expected by an option
 */
static int ret_arg_exp(const int mask) {
    int _ret_expected = FALSE;
    for (int i = 0; (i< (int)(sizeof(valid_options)/sizeof(struct opt_struct))); i++) {
        _ret_expected = (((valid_options[i].mask_val)== mask)||_ret_expected)? ((valid_options[i].opt_args)== 1)? TRUE : FALSE : FALSE ;
    }
    return _ret_expected;
}
/*! \brief val_opt_string is called from the validate_options() function and 
 * 	checks a given string if it matches the defined options
 * \retval FALSE if no match
 * \retval _arg_expected MASK of matched option
 */
static int ret_mask_to_opt_string(const char * arg) {
    int _return = FALSE;
    if(arg==NULL) {
        return FALSE;
    };
    for (int i = 0; (i< (int)(sizeof(valid_options)/sizeof(struct opt_struct))&&(!_return)); i++) {
        _return = (strcmp(valid_options[i].option,arg)==0)? (valid_options[i].mask_val) : FALSE ;
    }
    return _return;
}
/*! \brief ret_opt_on_mask
 * 		returns the optionstring to the given masked stored in opt_struct it is only called by main()
 *  \param mask integer that has to be checked if it matches the mask of option
 * 	\retval nvalid in case the mask has no option stored
 * 	\retval option in case the mask has an option
 */
static char *ret_opt_on_mask(const int mask) {
    errno = 0;
    char *_buf = malloc(sizeof(char)*MAX_OPTION_STRING_LENGTH+1);
    if(_buf == NULL) {
        print_error("error allocating memory","",errno);
        free(_buf);
        exit (EXIT_FAILURE);
    }
    for (int i = 0; (i< (int)sizeof(valid_options)/(int)sizeof(struct opt_struct)); i++) {
        if(valid_options[i].mask_val==mask) {
            strcpy(_buf,valid_options[i].option);
        }
    }

    return _buf;
}



/*! \brief compare_name
*   compares string to pattern via basename and returns matches
*   it uses fnmatch and in case of failure exits the program
*   \param file_name String that contains the filename to match the comparestrings criteria
*   \param compstring String that contains to information that the file has to
*/

static int compare_name(const char * file_name,const char * compstring){
    errno = 0;
    int _result = FALSE;

    if ((file_name == NULL) || (compstring == NULL)) {exit (EXIT_FAILURE);}

    char *_name_file_name = strdup(file_name);
	/*
	 * ### FB_RZ: hier sollte FNM_NOESCAPE als 3. Parameter stehen um \ im Namen
	 *            als ein normales Zeichen zu interpretieren.
	 */
    _result = fnmatch(compstring, basename(_name_file_name), 0);

    if ((_result != 0) && (_result != FNM_NOMATCH)) {
        print_error("error in fnmatch()(-name)",file_name,errno); 
        exit (EXIT_FAILURE);
    }
    free(_name_file_name); 
    if (_result == FNM_NOMATCH) {return FALSE;}
    if (_result == 0) {return TRUE;}
    exit (EXIT_FAILURE);

}



/*! \brief compare_type
 * 	checks the given string in compstring for validity and if valid compares the file's information with the type represented by compstring
*
* \param file_name String that contains the filename to match the comparestrings criteria
* \param file_info contains file_info
* \param compstring String that contains to information that the file has to
*
* \retval TRUE file matches criteria
* \retval FALSE file doesnt macht criteria
*/
static int compare_type(const char * file_name,struct stat *file_info, const char * compstring) {
    int _match = FALSE;
    if((1<strlen(compstring))) {
        /*only one letter expected, otherwise exit*/
        if(fprintf(stdout,"%s: Arguments to -type should contain only one letter\n",program)< 0){
			print_error("error in fprintf",file_name,errno);
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
    };
    for (int _index = 0; _index < TYPE_OPTIONS_CNT; _index++) {
        if ((strcmp(type_chars[_index].type,compstring)== 0)) {
            _match = TRUE;
            switch(type_chars[_index].type_val) {
            case BLOC_TYPE  :
                return S_ISBLK(file_info->st_mode)	? TRUE : FALSE;
                break;
            case CHAR_TYPE	:
                return S_ISCHR(file_info->st_mode)	? TRUE : FALSE;
                break;
            case DIR_TYPE	:
                return S_ISDIR(file_info->st_mode)	? TRUE : FALSE;
                break;
            case PIPE_TYPE	:
                return S_ISFIFO(file_info->st_mode)	? TRUE : FALSE;
                break;
            case FILE_TYPE	:
                return S_ISREG(file_info->st_mode)	? TRUE : FALSE;
                break;
            case LINK_TYPE	:
                return S_ISLNK(file_info->st_mode)	? TRUE : FALSE;
                break;
            case SOCK_TYPE	:
                return S_ISSOCK(file_info->st_mode)	? TRUE : FALSE;
                break;
            default :
                ;
                break;
            }
        }
        _match = FALSE;
    }
    if(_match==FALSE) {
        errno = 0;
        if(fprintf(stderr,"%s: Unknown argument to -type: %s\n",program,compstring)>0) {
            print_error("fprintf : Error writing to stderr",file_name,errno);
            exit(EXIT_FAILURE);
        }
    }
    return FALSE;
}
/*! \brief compare_user
 * checks the compstring for validity and if proceeding compares the string to the file informations user. if there is no match in name compstring is taken as uid and it checks again.
 * if the user asked for doesn't exist it exits the program after informing the user via stderr
*
*	\param file_name String that contains the filename. This is needed for print_error()
*   \param file_info contains stat info of file
*	\param compstring String that contains username or uid that the file has to match
*
*  \retval TRUE if the file belongs to the user or uid
*  \retval FALSE if the file belongs to someone else
*/
static int compare_user(const char * file_name,struct stat *file_info, const char * compstring) {
    if(compstring == NULL) {
        return FALSE;
    }
    /*!***init vars********/
	int _my_errno = 0;
    int _conv_uid = convert_str_to_uid(compstring); //  perform check if we have a user_id and not a user name
	uid_t _comp_uid = UINT_MAX;
    int _flag_uid = FALSE;
    int _flag_uname = FALSE;
	int _match_name = FALSE;
	int _match_uid = FALSE;
	char *_u_name = "";
	char *_comp_name = "";
	/*!********************************/
	/*check string in database-section*/
   	errno = _my_errno;                                                                                    /*check string in database section-explained*/
    struct passwd *_p_pwd_entry = getpwnam(compstring);                                                   /* perform checks on given string */
    if((_p_pwd_entry==NULL)&&(errno!=0)) {                                                                /* if we can get a match in the passwd */
        print_error("error in function getpwnam()",file_name,errno);                                      /* database and if so continue */
        exit (EXIT_FAILURE);                                                                              /* otherwise we will inform the user */
    };                                                                                                    /* that the searched for user is not */
	if(_p_pwd_entry == NULL&& errno == 0){ //user name doesn't exist                                      /* in the database */
		      _match_name = FALSE;
	}else{
		_match_name = TRUE;
		_comp_name = _p_pwd_entry->pw_name;
	}                                                                                                     
	if((_conv_uid >= 0)&&(_match_name == FALSE)){
		_p_pwd_entry = getpwuid(_conv_uid);
		if(_p_pwd_entry == NULL &&(errno != 0)){		                                                  
			print_error("error in function getpwuid()",file_name,errno);                                  
			exit (EXIT_FAILURE);                                                                          
		}                                                                                                 
		//user doesn't exist                                                                              
		if(_p_pwd_entry == NULL && (errno == 0)){                                                         
			                                                                                             
			_match_uid = FALSE;
		}else{
			_match_uid = TRUE;
			_comp_uid = _p_pwd_entry->pw_uid;
		}
	};
	if(!_match_name&&!_match_uid){
		if(fprintf(stderr,"%s: `%s' is not the name of a known user \n",program,compstring)<0){       
					print_error("fprintf: error writing to stdout",file_name,errno);                          
					exit(EXIT_FAILURE);                                                                       
		}
		exit (EXIT_FAILURE);
	}
	/*End check string in database-section*/
	/*!************************************/
	/*file-database-section*/
	errno = _my_errno;                                                       
    struct passwd *_file_pwd_entry = getpwuid(file_info->st_uid);            /*file-section explained */
	if(_file_pwd_entry == NULL && errno != 0){                               /* get files information */
		print_error("error in function getpwuid()",file_name,errno);         /*  so we can compare */
        exit (EXIT_FAILURE);                                                 /* it to strings information */
	}                                                                        
	if(_file_pwd_entry == NULL && errno == 0){                               
		/*no user owns the file !?*/                                         
		return FALSE;                                                        
	}
	/*End of file-database-section*/
    /*comparison section*/                                                  /*Comparison explained*/
	_u_name = _file_pwd_entry->pw_name;                                     /*if we passed that checks  owner of the file*/
    _flag_uname = strcmp(_u_name,_comp_name) == 0 ? TRUE : FALSE ;          /*we need to get the file's information to compare*/
	uid_t _u_uid = _file_pwd_entry->pw_uid;                                 /*if the user that is in the database is also the file's owner*/
	if (_match_name == FALSE){_flag_uid = (_u_uid == _comp_uid) ? TRUE : FALSE;}
    /*End of Comparison*/
	
    return (_flag_uid == TRUE || _flag_uname == TRUE) ? TRUE : FALSE;
}
/*! \brief convert_str_to_num
 *  converts a given string to a numerical value. This function is a helper function for compare_user() and it uses strtoll
 * \param string comparestring
 * 
 * \retval uid if string was a number
 * \retval -1 if string was number but too long for a uid
 * \retval -2 if string contained chars
*/
static long long convert_str_to_uid(const char *string){
	if(string == NULL){return -1;};
	char *_endptr = NULL;
	long long _uid;
	int _my_errno = 0;
	_uid =	strtoll(string,&_endptr,10);	//base is 10 because we want numbers only
	if(_uid == LLONG_MAX){
		_my_errno = errno;
		print_error("error in strtoll():","Unknown",_my_errno);
		exit (EXIT_FAILURE);
	}
	if(_uid == 0 && (strcmp(string,"0")!= 0)) {_uid = -2;}; //if we search for root but with a digit
	return (strcmp(_endptr,string)!=0)? (_uid >  UINT_MAX)? -1 : _uid : -2 ; // -1 if uid is longer than a uid can be and -2 if it contained chars
}
/***********************EOF************************************/
