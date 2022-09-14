#include "gh_core.h"

gh_hed::gh_hed(FILE * _fp)
{
	fseek(_fp, 0, SEEK_SET);
	unsigned int kind;
	fread(& kind, 4, 1, _fp);

	int shift = kind == 1179535682 ? 0x214 : 0x254;
	int tag_size = kind == 1179535682 ? 9 : 40;

	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		fseek(_fp, shift, SEEK_SET);
		fread(& size[i], 2, 1, _fp);
		size[i] &= 0xffff;
		shift += 12;
		
		//DEBUG("the size of dimension #%d is %d\n", i, size[i]);
		
	}

	att = new char *** [gh::dim_qua];
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		att[i] = new char ** [size[i]];
		for(unsigned int j(0); j < size[i]; j++)
		{
			att[i][j] = new char * [3];
			att[i][j][0] = new char [tag_size + 1];
			att[i][j][1] = new char [gh::sht_size + 1];
			att[i][j][2] = new char [gh::lng_size + 1];
		}
	}

	//DEBUG("Hi 1\n");
	
	int ja;
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		for(unsigned int j(0); j < size[i]; j++)
		{
			ja = i == gh::dim_qua - 1 ? size[i] - j - 1 : j;
			fread(att[i][ja][0], tag_size, 1, _fp);
			att[i][ja][0][tag_size] = '\0';
			fseek(_fp, 3, SEEK_CUR);//tag gap
		}
	}

	//DEBUG("Hi 2\n");

	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		for(unsigned int j(0); j < size[i]; j++)
		{
			ja = i == gh::dim_qua - 1 ? size[i] - j - 1 : j;

			fread(att[i][ja][2], gh::lng_size, 1, _fp);
			att[i][ja][2][gh::lng_size] = '\0';

			fread(att[i][ja][1], gh::sht_size, 1, _fp);
			att[i][ja][1][gh::sht_size] = '\0';
		}
	}

	//DEBUG("Hi 3\n");

}

gh_hed::~gh_hed()
{
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		for(unsigned int j(0); j < size[i]; j++)
		{
			for(int k(0); k < 3; k++)
			{
				delete [] att[i][j][k];
			}
			delete [] att[i][j];
		}
		delete [] att[i];
	}
	delete [] att;
}

unsigned int gh_hed::get(unsigned int _dim_num, const char * _tag)
{
	if(_dim_num >= gh::dim_qua)
	{
		ERROR
		printf("header: There is no dimension # %d in database...\n\n", _dim_num);
		return gh::wrong_number;
	}
	for(unsigned int i(0); i < size[_dim_num]; i++) if(!strcmp(_tag, att[_dim_num][i][0])) return i;
	ERROR
	printf("header: There is no tag %s in dimension # %d...\n\n", _tag, _dim_num);
	return gh::wrong_number;
}

unsigned int gh_hed::get(unsigned int _dim_num, unsigned int _item_num)
{
	//DEBUG("_dim_num: %d,_item_num: %d \n", _dim_num,_item_num);

	if(_dim_num >= gh::dim_qua)
	{
		ERROR
		printf("header: There is no dimension # %d in database...\n\n", _dim_num);
		return gh::wrong_number;
	}
	if((_item_num < 0) || (_item_num >= size[_dim_num]))
	{
		ERROR
		printf("header: There is no item %d in dimension # %d...\n\n", _item_num, _dim_num);
		return gh::wrong_number;
	}
	//DEBUG("Hi there... dim: %d, item: %d\n", _dim_num, _item_num);
	return _item_num;
}

void gh_hed::log(const char * _hed_name)
{
	char log_name[350];
	strcpy(log_name, _hed_name);
	char * a = 0;
	while((a = strchr(log_name, '/'))) * a = '_';
	if((a = strrchr(log_name, '.'))) * a = '\0';
	strcat(log_name, ".hed.log");
	FILE * fp = fopen(log_name, "w");
	if(!fp)
	{
		ERROR;
		printf("gh_core:Cannot find the \"./rep\" directory\n");
		return;
	}
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		for(unsigned int j(0); j < size[i]; j++)
		{
			fprintf(fp, "%7d %-11s  %-41s %-80s\n", j, att[i][j][0], att[i][j][1], att[i][j][2]);
		}
	}
	fclose(fp);
}
