#include "gh_core.h"

gh_core::gh_core(const char * _hed_name, bool _is_opt, bool _is_logs)
{
	integrity = conn_db(_hed_name, _is_opt);
	
	switch(integrity)
	{
//		case 2 :
//		{
//			for(unsigned int i = 0; i < gh::dim_qua; i++) hie->dim_size[i] = hed->size[i];
//			if(_is_logs) hie_log(_hed_name);
//		}
		case 1 :
		{
			//DEBUG("Start log\n");
			if(_is_logs) hed->log(_hed_name);
			//DEBUG("Stop log\n");
			break;
		}
		default:
		{
			ERROR
			printf("gh_core: Cannot connect to data source %s\n", _hed_name);
		}
	}
}

gh_core::~gh_core()
{
//	switch(integrity)
//	{
//		case 2 : delete hie;
//		case 1 :
//		{
//			delete inf;
//			delete idx;
			delete hed;
//		}
//	}
}

unsigned int gh_core::conn_db(const char * _file_name, bool _is_opt)
{
	char name[256];
	strcpy(name, _file_name);
	char * a = strstr(name, ".hed");
	if(!a) return 0;
	* a = '\0';

	char hed_name[256];
	strcpy(hed_name, name);
	strcat(hed_name, ".hed");
	FILE * fp_hed = fopen(hed_name, "rb");
	if(!fp_hed) return 0;
	
	//DEBUG("hed done\n");
	
//	char idx_name[256];
//	strcpy(idx_name, name);
//	strcat(idx_name, ".idx");
//	FILE * fp_idx = fopen(idx_name, "rb");
//	if(!fp_idx) return 0;
	
	//DEBUG("idx done\n");

//	char inf_name[256];
//	strcpy(inf_name, name);
//	strcat(inf_name, ".inf");
//	FILE * fp_inf = fopen(inf_name, "rb");
//	if(!fp_inf) return 0;
	
	//DEBUG("inf done\n");

	hed = new gh_hed(fp_hed);
	fclose(fp_hed);
	return 1;
	
	//DEBUG("idx switch 1\n");
//	idx = _is_opt ? new gh_idx(fp_idx, hed->size) : new gh_idx(fp_idx);
//	inf = new gh_inf(fp_inf);
	
	//DEBUG("idx switch 2\n");

//	char chr_name[256];
//	strcpy(chr_name, name);
//	strcat(chr_name, ".chr");
//	FILE * fp_chr = fopen(chr_name, "rb");
//	if(!fp_chr) return 1;

//	char tad_name[256];
//	strcpy(tad_name, name);
//	strcat(tad_name, ".tad");
//	FILE * fp_tad = fopen(tad_name, "rb");
//	if(!fp_tad) return 1;

//	hie = new gh_hie(fp_chr, fp_tad);

	//DEBUG("Hi there!\n");

//	return 2;
}

//const char * gh_core::get_attr_by_num(unsigned int _dim_num, unsigned int _att_num, unsigned int _item_num)
//{
//	if(_dim_num >= gh::dim_qua) return 0;
//	if(_att_num > 2) return 0;
//	if((_item_num < 0) || (_item_num >= hed->size[_dim_num])) return 0;
//	return hed->att[_dim_num][_item_num][_att_num];
//}


///////////////////////////////////////////////////////////////////////////////
//void gh_core::hie_log(const char * _hed_name)
//{
//	char log_name[350];
//	strcpy(log_name, _hed_name);
//	char * a = 0;
//	while((a = strchr(log_name, '/'))) * a = '_';
//	if((a = strrchr(log_name, '.'))) * a = '\0';
//	strcat(log_name, ".hie.log");
//	FILE * fp = fopen(log_name, "w");
//	if(!fp)
//	{
//		ERROR;
//		printf("gh_core:Cannot open \"hie.log\" file\n");
//		return;
//	}
//	for(unsigned int i(0); i < gh::dim_qua; i++)
//	{
//		fprintf(fp, "********************** Dimension:%d **********************\n", i);
//		for(unsigned int j(0); j < hie->seg_qua[i]; j++)
//		{
//			fprintf(fp, "Segment: %3d %s\n", j, hie->seg[i][j]);
//			for(unsigned int k(0); k < hie->off_qua[i][j]; k++)
//			{
//				fprintf(fp, "\tOffset: %5d :%-82s\n", k, hie->off[i][j][k]);
//			}
//		}
//	}
//	fclose(fp);
//}

