#define DEBUG printf

#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>
#include "gh_core.h"


int main(int argc, char ** argv)
{
	gh_core * core;

	core = new gh_core((const char *) "/home/ilya/infact/RSTEARI.hed", false, true);
// 	core = new gh_core((const char *) "/home/ilya/infact/RSCOLRI.hed", true, true);
// 	core = new gh_core((const char *) "/home/ilya/infact/RSBDRRI.hed", true, true);
	delete core;

	return 0;
}


	//gh_core * core = new gh_core((const char *) "/mnt/cip/RSMAYRI.hed", false, true);

	
	//core->get(1, 1, 1, 1);
	
	
	
// 	unsigned int dim_num = 1;
// 	unsigned int hie_num = 2;
// 
// 	unsigned int hie_size = core->get_off_qua(dim_num, hie_num);
// 
// 	unsigned int * a = core->get_hie_segs(dim_num, hie_num);
// 	for(unsigned i = 0; i < hie_size; i++)
// 	{
// 		printf("%d\n", a[i]);
// 	}

// 	unsigned int dim_num = 1;
// 	unsigned int size;
// 	unsigned int coo = 22;
// 	//unsigned int coo = 9;
// 
// 	char ** parents = core->get_info(dim_num, coo, size);
// 	
// 	for(unsigned int i = 0; i < size; i++)
// 	{
// 		printf("%s\n", parents[i]);
// 	}
// 
// 	for(unsigned int j = 0; j < size; j++) delete [] parents[j];
// 	delete [] parents;





	//FILE * fp_rep = fopen("./rep", "w");
	//gh_core * core = new gh_core((const char *) "/mnt/wss/RSOLVRI.hed", false, true);

// 	fclose(fp_rep);













// int main(int argc, char ** argv)
// {
// 	FILE * fp_rep = fopen("./rep", "w");
// 	gh_core * core = new gh_core((const char *) "/mnt/wss/NSDTRNT.hed", true, true);
// 
// 	unsigned int size;
// 	unsigned int coo = 0;
// 	char ** parents = core->get_info(1, coo, size);
// 	if(parents)
// 	{
// 		for(unsigned int j = 0; j < size; j++)
// 		{
// 			printf("%s\t", parents[j]);
// 		}
// 		printf("\n");
// 
// 		for(unsigned int j = 0; j < size; j++) delete [] parents[j];
// 		delete [] parents;
// 	}
// 
// 	parents = core->get_info(1, coo, size);
// 	if(parents)
// 	{
// 		for(unsigned int j = 0; j < size; j++)
// 		{
// 			printf("%s\t", parents[j]);
// 		}
// 		printf("\n");
// 
// 		for(unsigned int j = 0; j < size; j++) delete [] parents[j];
// 		delete [] parents;
// 	}
// 
// 	parents = core->get_info(1, coo, size);
// 	if(parents)
// 	{
// 		for(unsigned int j = 0; j < size; j++)
// 		{
// 			printf("%s\t", parents[j]);
// 		}
// 		printf("\n");
// 
// 		for(unsigned int j = 0; j < size; j++) delete [] parents[j];
// 		delete [] parents;
// 	}
// 	
// 	
// 	
// 	
// 
// 	delete core;
// 	fclose(fp_rep);
// 
// 	return 0;
// }




// 	//double * figs = core->get(405, 0, 0);
// 	//double * figs = core->get("M000000000000106327900000000000001113024", "P000000000000612698100000000009622273921", "F000000000000000000100000000000000000000");
// 
// 	unsigned int target_level = 1;
// 	unsigned int dim_num = 1;
// 
// 
// 	unsigned int coo = 372;
// 
// 	unsigned int * ch = core->get_children_from_level_3(dim_num, coo, target_level);
// 
// 	//DEBUG
// 	//(
// 	//	"hie: %d, seg: %d, hie_seg: %d, off: %d, segs_hie: %d\n", 
// 	//	coo_bind::hie, coo_bind::seg, coo_bind::hie_seg, coo_bind::off, coo_bind::segs_hie
// 	//);
// 	
// 
// 	if(ch)
// 	{
// 		//DEBUG("\nThe number of ch is = %d\n\n", ch[0]);
// 
// 		for(unsigned int i = 1; i <= ch[0]; i++)
// 		{
// 			fprintf(fp_rep, "%6d\t%s\n", ch[i], core->get_attr_by_num(dim_num, 1, ch[i]));
// 		}
// 	}
