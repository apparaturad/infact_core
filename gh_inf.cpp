#include "gh_core.h"

gh_inf::gh_inf(FILE * _fp)
{
	fp = _fp;
}

gh_inf::~gh_inf()
{
	fclose(fp);
}

void gh_inf::fill_nulls(unsigned short _vis_size)
{
	for(int i(0); i < _vis_size; i++) digs[i] = 0.0;
}

void gh_inf::read_digs(unsigned short _vis_size, unsigned short _length, unsigned short _prec, bool _full)
{
	//DEBUG("_vis_size: %d, _length: %d, _prec: %d, _full: %d\n", _vis_size, _length, _prec, _full);
	
	
	
	int j;
	int logar = 1;
	unsigned int mask = 0;
	//unsigned long long gh_dig;
	unsigned int gh_dig = 0;
	for(int i(0); i < _prec; i++) logar *= 10;
	for(int i(0); i < _length; i++) mask = mask * 0x100 + 0xff;
	for(int i(0); i < _vis_size; i++)
	{
		j = i;
		if(!_full)
		{
			fread(& gh_dig, 1, 1, fp);
			j = (gh_dig & 0xff) - 1;
			//DEBUG("%d\n", j);
		}
		if(_length == 8)
		{
			fread(& digs[j], _length, 1, fp);
			digs[j] /= logar;
			//DEBUG("L8: %f\n", digs[j]);
		}
		else
		{
			fread(& gh_dig, _length, 1, fp);
			bool neg = gh_dig & 0x80000000;
			if(neg) gh_dig = (~ gh_dig) + 1;
			gh_dig &= mask;

			digs[j] = gh_dig;
			digs[j] /= logar;

			if(neg) digs[j] *= -1;


			//DEBUG("L: %15.4f\tlogar: %d\tmask:%X\t\tdig: %X\n", digs[j], logar, mask, gh_dig);
		}
	}
}

double * gh_inf::get(unsigned long _addr, unsigned short _vis_size)
{
	//DEBUG("				addr: %x\n", _addr);
	
	
	//unsigned short vis_cnt, prec, length;
	//unsigned short prec, length;
	unsigned int vis_cnt = 0xff;
	unsigned int prec = 0;
	unsigned int length = 0;


	digs = new double[gh::per_max];

	
	
	
	fseek(fp, _addr, SEEK_SET);
	fseek(fp, 6, SEEK_CUR);

	fread(& vis_cnt, 1, 1, fp);

	//DEBUG("vis_cnt: %x\n", vis_cnt);
	
	vis_cnt &= 0xff;

	//DEBUG("vis_cnt: %x\n", vis_cnt);
	

	fread(& prec, 1, 1, fp);
	prec &= 0xff;

	fread(& length, 1, 1, fp);
	length &= 0xff;

	//DEBUG("vis_cnt: %x\n", vis_cnt);
	
	switch(vis_cnt)
	{
		case 0xff :
		{
			read_digs(_vis_size, length, prec, true);
			break;
		}
		case 0 :
		{
			fill_nulls(_vis_size);
			break;
		}
		default:
		{
			fill_nulls(_vis_size);
			read_digs(vis_cnt, length, prec, false);
		}
	}
	return digs;
}
