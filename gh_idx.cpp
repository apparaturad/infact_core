#include "gh_core.h"

gh_idx::gh_idx(FILE * _fp, unsigned int * _dim_size)
{
	dim_size = _dim_size;
	fp = _fp;

	// определяем размер адреса в inf файле: 4 или 8 байт для файла inf более 4Гб размером.
	unsigned short inf_size_code;
	fseek(fp, 0, SEEK_SET);
	fread(& inf_size_code, 2, 1, fp);
	
	
	if(inf_size_code == 0x61) inf_addr_size = 4;
	else if(inf_size_code == 0x62) inf_addr_size = 8;
	else inf_addr_size = 0; //FIXME сообщение об ошибке

	//DEBUG("value is: %d\n", inf_addr_size);

	idx_to_array();
}

gh_idx::gh_idx(FILE * _fp)
{
	dim_size = 0;
	fp = _fp;

	// определяем размер адреса в inf файле: 4 или 8 байт для файла inf более 4Гб размером.
	unsigned short inf_size_code;
	fseek(fp, 0, SEEK_SET);
	fread(& inf_size_code, 2, 1, fp);
	if(inf_size_code == 0x61) inf_addr_size = 4;
	else if(inf_size_code == 0x62) inf_addr_size = 8;
	else inf_addr_size = 0; //FIXME сообщение об ошибке

	//DEBUG("value is: %d\n", inf_addr_size);
}

gh_idx::~gh_idx()
{
	fclose(fp);
	if(dim_size != 0)
	{
		for(unsigned int i(0); i <= dim_size[0]; i++)
		{
			for(unsigned int j(0); j <= dim_size[1]; j++)
			{
				delete [] array[i][j];
			}
			delete [] array[i];
		}
		delete [] array;
	}
}

unsigned long gh_idx::get(unsigned short _mkt, unsigned short _prd, unsigned short _fct)
{
	if(dim_size != 0) return(array[_mkt][_prd][_fct]);

	reverse(++_mkt);
	reverse(++_prd);
	reverse(++_fct);

	unsigned long dom_addr = gh::idx_start_point;
	while(get_dom_data(_mkt, _prd, _fct, dom_addr) == 0);
	
	return dom_addr;
}

int gh_idx::get_dom_data(unsigned short _mkt, unsigned short _prd, unsigned short _fct, unsigned long & _dom_addr)
{
	unsigned short dom_size;
	unsigned short last_lev;
	unsigned short mkt, prd, fct;

	fseek(fp, _dom_addr + 8, SEEK_SET);
	fread(& dom_size, 2, 1, fp);
	fseek(fp, 2, SEEK_CUR);
	fread(& last_lev, 2, 1, fp);
	for(int i(0); i < dom_size; i++)
	{
		fread(& _dom_addr, inf_addr_size, 1, fp);

		//_dom_addr &= 0x7fffffff;


		fread(& mkt, 2, 1, fp);
		fread(& prd, 2, 1, fp);
		fread(& fct, 2, 1, fp);

		reverse(mkt);
		reverse(prd);
		reverse(fct);

		if(mkt > _mkt) break;
		if(mkt == _mkt)
		{
			if(prd > _prd) break;
			if(prd == _prd && fct >= _fct) break;
		}
	}
	if(last_lev > 0)
	{
		if(mkt != _mkt || prd != _prd || fct != _fct) _dom_addr = 0;
		return 1;
	}
	else return 0;
}

void gh_idx::reverse(unsigned short & _i)
{
	_i = ((_i & 0xff00) >> 8) | ((_i & 0xff) << 8);
}

void gh_idx::idx_to_array()
{
	unsigned int point = 0;// debug
	//unsigned int first_addr = 0;// debug
	//unsigned int last_addr = 0;// debug
	
	//int start_point = 0x200;
	int dom_addr = gh::idx_start_point;

	array = new unsigned long ** [dim_size[0] + 1];
	for(unsigned long i(0); i < dim_size[0] + 1; i++)
	{
		array[i] = new unsigned long * [dim_size[1] + 1];
		for(unsigned long j(0); j < dim_size[1] + 1; j++)
		{
			array[i][j] = new unsigned long [dim_size[2] + 1];
			for(unsigned long k(0); k < dim_size[2] + 1; k++) array[i][j][k] = .0;
		}
	}

	unsigned long addr;
	unsigned int flag;
	unsigned short dom_size;
	unsigned short mkt, prd, fct;

	while(dom_addr != 0)
	{
		//DEBUG("dom addr is: %x\n", dom_addr);
		fseek(fp, dom_addr, SEEK_SET);
		fread(& dom_addr, 4, 1, fp);
		fseek(fp, 4, SEEK_CUR);
		//fread(& last_addr, 4, 1, fp); // debug
		fread(& dom_size, 2, 1, fp);
		fread(& flag, 4, 1, fp);

		for(int i(0); i < dom_size; i++)
		{
			//inf_4gb == 1 ? fread(& addr, 4, 1, fp) : fread(& addr, 8, 1, fp);
			fread(& addr, inf_addr_size, 1, fp);
			fread(& mkt, 2, 1, fp);
			fread(& prd, 2, 1, fp);
			fread(& fct, 2, 1, fp);

			if(dom_addr == 0) if(mkt > dim_size[0] || prd > dim_size[1] || fct > dim_size[3]) break;
			
			array[--mkt][--prd][--fct] = addr;
			
			point++;// debug
		}
	}

	DEBUG("Points number forward is: %d\n", point);

	// Назад до первого
	dom_addr = gh::idx_start_point;
	while(dom_addr != 0)
	{
		//DEBUG("dom addr back is: %x\n", dom_addr);
		fseek(fp, dom_addr, SEEK_SET);
		fseek(fp, 4, SEEK_CUR);
		//fread(& first_addr, 4, 1, fp); // debug
		fread(& dom_addr, 4, 1, fp);
		fread(& dom_size, 2, 1, fp);
		fread(& flag, 4, 1, fp);

		for(int i(0); i < dom_size; i++)
		{
			//inf_4gb == 1 ? fread(& addr, 4, 1, fp) : fread(& addr, 8, 1, fp);
			fread(& addr, inf_addr_size, 1, fp);
			fread(& mkt, 2, 1, fp);
			fread(& prd, 2, 1, fp);
			fread(& fct, 2, 1, fp);

			if(dom_addr == 0) if(mkt > dim_size[0] || prd > dim_size[1] || fct > dim_size[3]) break;

			array[--mkt][--prd][--fct] = addr;
			
			point++;// debug
		}
	}

	//DEBUG("Points number forward is: %d, the first addr is: %x\n", point, first_addr);
	DEBUG("Points number back is: %d\n", point);
}
