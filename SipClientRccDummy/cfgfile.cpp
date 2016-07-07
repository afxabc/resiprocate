

#if defined( WIN32 )
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h> 
#endif
#include <stdio.h>


static int l_trim(char* str)
{
	int i, p = 0;
	bool bstart = false;

	int len = strlen(str);

	for (i = 0; i<len; i++)
	{
		if (!bstart)
		{
			if (!isspace((unsigned char)(str[i])))
				bstart = true;
			else p++;
		}

		if (bstart)
			str[i - p] = str[i];
	}

	str[i - p] = str[i];

	return strlen(str);
}

static int r_trim(char* str)
{
	int len = strlen(str);

	for (int i = len - 1; i >= 0; i--)
	{
		if (!isspace((unsigned char)(str[i])))
			break;
		else str[i] = 0;
	}

	return strlen(str);
}

int LoadParamString(const char* profile, char* AppName, char* KeyName, char* KeyVal)
{
	static const int KEYVALLEN = 512;

	char appname[128], keyname[128];
	char buf[KEYVALLEN], *c;
	FILE *fp;

	int found = 0; /* 1 AppName 2 KeyName */

	if ((fp = fopen(profile, "r")) == NULL)
	{
		return(-1);
	}

	fseek(fp, 0, SEEK_SET);
	sprintf(appname, "[%s]", AppName);
	memset(keyname, 0, sizeof(keyname));

	while (!feof(fp) && fgets(buf, KEYVALLEN, fp) != NULL)
	{
		if (l_trim(buf) <= 0)
			continue;

		if (found == 0)
		{
			if (buf[0] != '[')
			{
				continue;
			}
			else if (strncmp(buf, appname, strlen(appname)) == 0)
			{
				found = 1;
				continue;
			}
		}
		else if (found == 1)
		{
			if (buf[0] == '#')
			{
				continue;
			}
			else if (buf[0] == '[')
			{
				break;
			}
			else
			{
				if ((c = (char*)strchr(buf, '=')) == NULL)
					continue;
				memset(keyname, 0, sizeof(keyname));
				sscanf(buf, "%[^=]", keyname);
				r_trim(keyname);
				if (strcmp(keyname, KeyName) == 0)
				{
					sscanf(++c, "%[^\n]", KeyVal);
					l_trim(KeyVal);
					r_trim(KeyVal);
					found = 2;
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}

	fclose(fp);

	return found;
}
