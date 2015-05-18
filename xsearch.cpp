#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#define _DEBUG

#define USAGE printf("usage:\e[1mxsearch\e[0m \e[4mDIRECTORY\e[0m \e[4mKEYWORDS\e[0m \e[4m-i IGNORE\e[0m \e[4m-t FILETYPE\e[0m\nexample:./xsearch . 653377 -i .svn -t cpp\n"); return -1;

static char szBase[512] = {0};
static char szKey[512] = {0};

char** g_ignore_list = NULL;
int g_ignore_list_size = 0;

char** g_file_type_list = NULL;
int g_file_type_list_size = 0;

int search_inline(char* file, char* line, int nline)
{
    int ret = -1;
    
    char* p = strstr(line, szKey);
    if(p == NULL)
    {
        return ret;
    }
    //printf("\e[34m[%s-%d]\e[0m%s", file, nline+1, line);
    printf("\e[34m\e[1m[%s:%d]\e[0m", file, nline+1);
    
    char* pTemp = line;
    char* pTail = (p + strlen(szKey));
    while(p)
    {
        while(pTemp != p)
        {
            printf("%c", *pTemp);
            pTemp ++;
        }
        printf("\e[31m\e[1m%s\e[0m", szKey);
        
        p += strlen(szKey);
        pTail = pTemp = p;

        p = strstr(p, szKey);
    }
	//TODO if pTail contains some format string, such as ", \ %u, %d, might got SIGSEGV
    //printf(pTail);
	std::cout<<pTail;
    return ret;
}

bool check_file_type(const char* name)
{
	if(g_file_type_list_size == 0)
	{
		return true;
	}
	
	//get file type
	size_t _len = strlen(name);
	if(_len == 0)
	{
		return false;
	}

	const char* _p = name + _len - 1;
	while(_p != name)
	{
		if(*_p == '.')
		{
			break;
		}
		_p--;
	}	
	_p++;
	//printf("type:%s\n", _p);

        int i=0;
        for(;i<g_file_type_list_size; i++)
        {
                if(g_file_type_list[i] && strcmp(g_file_type_list[i], _p) == 0)
                {
                        return true;
                }
        }
	

	return false;
}

bool check_ignore(const char* name)
{
	if(name == NULL)
	{
		return false;
	}

	int i=0;
	for(;i<g_ignore_list_size; i++)
	{
	        if(g_ignore_list[i] && strcmp(g_ignore_list[i], name) == 0)
		{
			return false;
		}
	}

	return true;
}

int search_infile(char* szfile)
{
	//printf("file:%s\n", szfile);
        if(check_file_type(szfile) == false)
        {
		//printf("XXXXXXXXXufcked.\n");
                return 0;
        }

    int ret = -1;

    FILE*    file = fopen(szfile, "r");
    char*     line = NULL;

    if(file != NULL)
    {
        int n = 0;
        size_t size;
        while(getline(&line, &size, file) != -1)
        {
            search_inline(szfile, line, n);
            n++;
        }

        free(line);
        fclose(file);
    }
#ifdef _DEBUG
    else
    {
        printf("open file \'%s\' failed!%s\n", szfile, strerror(errno));
    }
#endif
    return ret;
}

int search_indir(char* dir)
{
    struct dirent** namelist;
    int n = scandir(dir, &namelist, 0, 0);
    if(n < 0)
    {
        perror("error in scandir\n");
        return -1;
    }
    
    while(n--)
    {
        if(strcmp(".", namelist[n]->d_name)== 0
        ||strcmp("..", namelist[n]->d_name) == 0)
        {
            //printf("ignored!%s\n", namelist[n]->d_name);
            continue;
        }

	if(check_ignore(namelist[n]->d_name) == false)
	{
		continue;
	}

        char szTemp[512] = {0};
        sprintf(szTemp, "%s/%s", dir, namelist[n]->d_name);
        //printf("searching:%s\n", szTemp);        

        if(namelist[n]->d_type == DT_REG)
        {
            search_infile(szTemp);
        }
        else if(namelist[n]->d_type == DT_DIR)
        {
            search_indir(szTemp);
        }
        free(namelist[n]);
    }
    free(namelist);
}

char** split_param_list(const char* param, const char* delim, int* size)
{
	if(param == NULL || delim == NULL || size == NULL)
	{
		return NULL;
	}
	//printf("%s\n", param);

	char** list = NULL;
	int _len = strlen(param);
	*size = 0;

	int i=0;
	for(; i<_len; i++)
	{
		if(param[i] == *delim)
		{
			(*size)++;
		}
	}	
	(*size)++;

	list = (char**)malloc(*size);

	int _count = 0;
	const char* _start=param;
	for(i=0; i<_len; i++)
	{
		if(param[i] == *delim)
		{
			const char* _p = param + i;
			if(_p-_start == 0)
			{
				_start++;
				continue;
			}

			list[_count] = (char*)malloc(_p-_start);
			strncpy(list[_count], _start, _p-_start);
			
			_start = ++_p;
			_count++;
		}
	}
	
	if(*_start != *delim)
	{
		const char* _p = param + i;
                list[_count] = (char*)malloc(_p-_start);
                strncpy(list[_count], _start, _p-_start);
	}

	//dump
	/*for(i=0; i<*size; i++)
	{
		printf("%s\n", list[i]);
	}
	*/
	return list;
}

void test()
{
	split_param_list("sdvs,gdfa,cd,a232,fuck",",", &g_ignore_list_size);
	exit(0);
}

int main(int argc, char** argv)
{
	//test();

    if(argc < 3)
    {
        USAGE;
    }

    strcpy(szBase, argv[1]);
    
    //keyworks fromat: -'KEY WORDS', will search KEY WORDS
    if((strncmp(argv[2], "\'", 2) == 0) && (strlen(argv[2]) > 3))
    {
        char* p = argv[2] + 2;
        strncpy(szKey, p, strlen(p)-1);
        printf("searching:%s\n", szKey);
    }
    else
    {
        strcpy(szKey, argv[2]);
    }

	
	if(argc > 3)
	{
		for(int i=3; i<argc; i++)
		{
			if(strcmp("-i", argv[i]) == 0)
			{
				if(i == argc -1)
				{
					USAGE;
				}
				g_ignore_list =	split_param_list(argv[++i], ",", &g_ignore_list_size);
			}
			else if(strcmp("-t", argv[i]) == 0)
			{                           
				if(i == argc -1)
                                {
                                        USAGE;
                                }
				g_file_type_list = split_param_list(argv[++i], ",", &g_file_type_list_size);
				
			}
		}
	}

	printf("************************************************\n");
	if(g_file_type_list_size > 0)
	{
		printf("\n[file type]:\n");
		int i=0;
		for (;i<g_file_type_list_size; i++)	
		{
			printf("%s\n", g_file_type_list[i]);
		}
	}

        if(g_ignore_list_size > 0)
        {
                printf("\n[ignore]:\n");
                int i=0;
                for (;i<g_ignore_list_size; i++)
                {
                        printf("%s\n",g_ignore_list[i]);
                }
        }
	printf("************************************************\n");

    search_indir(szBase);
        
    return 0;
}
