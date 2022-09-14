#include "gh_core.h"
#include "gh_coo.h"

gh_hie::gh_hie(FILE * _fp_chr, FILE * _fp_tad)
{
	fp_chr = _fp_chr;
	fp_tad = _fp_tad;
	dim_num = 0;
	broken_item_cnt = 0;
	dim_shift = 0;
	//seg_num = 0;
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		sp[i] = 0;
		sp_nm[i] = 0;
	}
	seg_qua = new unsigned int[gh::dim_qua];
	for(unsigned int i(0); i < gh::dim_qua; i++) seg_qua[i] = 0;
	seg = new char ** [gh::dim_qua];
	off_qua = new unsigned int * [gh::dim_qua];
	off = new char *** [gh::dim_qua];
	mask_qua = new unsigned int[gh::dim_qua];
	masks = new unsigned int *** [gh::dim_qua];
	stack = new unsigned int ** [gh::dim_qua];
	new_masks = new unsigned int ** [gh::dim_qua];
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		stack[i] = new unsigned int * [gh::stack_size];
		new_masks[i] = new unsigned int * [gh::stack_size];
	}

	//DEBUG("Cha:cha\n");

	run_cha_domens();
}

gh_hie::~gh_hie()
{
	fclose(fp_chr);
	fclose(fp_tad);
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		delete [] stack[i];
		delete [] new_masks[i];
	}
	delete [] stack;
	delete [] new_masks;

	for(unsigned int i = 0; i < gh::dim_qua; i++)
	{
		if(seg_qua[i] == 0) continue;
		for(unsigned int j = 0; j < seg_qua[i]; j++)
		{
			for(unsigned int k = 0; k < off_qua[i][j]; k++)
			{
				delete [] off[i][j][k];
				delete [] masks[i][j][k];
			}
			delete [] off[i][j];
			delete [] seg[i][j];
			delete [] masks[i][j];
		}
		delete [] off[i];
		delete [] off_qua[i];
		delete [] seg[i];
		delete [] masks[i];
	}
	delete [] off;
	delete [] off_qua;
	delete [] seg;
	delete [] seg_qua;
	delete [] masks;
	delete [] mask_qua;
}

void gh_hie::run_cha_domens()
{
	unsigned int next_domen_addr(0);
	unsigned int curr_domen_addr(0);
	unsigned int prev_domen_addr(0x200);
	do
	{
		curr_domen_addr = prev_domen_addr;
		fseek(fp_chr, curr_domen_addr, SEEK_SET);
		fread(& next_domen_addr, 4, 1, fp_chr);
		next_domen_addr &= 0xffffffff;
		fread(& prev_domen_addr, 4, 1, fp_chr);
		prev_domen_addr &= 0xffffffff;
	}
	while(prev_domen_addr > 0);
	next_domen_addr = curr_domen_addr;
	unsigned int this_domen_size;
	do
	{
		fseek(fp_chr, next_domen_addr, SEEK_SET);
		fread(& next_domen_addr, 4, 1, fp_chr);
		next_domen_addr &= 0xffffffff;
		fseek(fp_chr, 4, SEEK_CUR);
		fread(& this_domen_size, 2, 1, fp_chr);
		this_domen_size &= 0xffff;
		fseek(fp_chr, 4, SEEK_CUR);
		if(!read_cha_domen(this_domen_size)) break;
	}
	while(next_domen_addr > 0);
}

bool gh_hie::read_cha_domen(unsigned int _domen_size)
{
	//DEBUG("read_cha_domen_in\n");

	unsigned int addr_tad;
	unsigned int seg_num;
	unsigned int off_num;

	for(unsigned int i(0); i < _domen_size; i++)
	{
		fread(& addr_tad, 4, 1, fp_chr);
		addr_tad &= 0xffffffff;
		
		//DEBUG("\naddr_tad:%d\n", addr_tad);

		fread(& off_num, 2, 1, fp_chr);

		//DEBUG("off_num befor:%10d\t%10x\n", off_num, off_num);
		
		(off_num & 0x8000) ? off_num = 0 : off_num &= 0xffff;	//	FIXME

		//DEBUG("off_num after:%10d\t%10x\n\n", off_num, off_num);
		
		seg_num = 0;
		fread(& seg_num, 1, 1, fp_chr);
		
		//DEBUG("\nseg_num before: %12d\t%10x\n", seg_num, seg_num);
		
		seg_num &= 0xff;

		//DEBUG("seg_num after: %13d\t%10x\n", seg_num, seg_num);

		if(seg_num == 0xff) return false;
		//if(seg_num == 0xff) break;//FIXME
		//if(seg_num == 0xfe) return false;//FIXME
		else if(seg_num == 0) read_seg(addr_tad, off_num);
		else
		{
			//DEBUG("Hi there 1\n");
			
			seg_num--;
			//seg_num++;
			if(seg_num == seg_qua[dim_num] + dim_shift)
			{
				dim_shift += seg_qua[dim_num];
				dim_num++;
				
				//DEBUG("dim_shift: %d\n", dim_shift);
				
				while(seg_qua[dim_num] == 0 && dim_num < gh::dim_qua) dim_num++;
			}
			
			//DEBUG("Hi there 2\n");
			
			//DEBUG("seg_num before: %12d\t%10x\n", seg_num, seg_num);

			seg_num -= dim_shift;
			
			//DEBUG("seg_num after: %13d\t%10x\n", seg_num, seg_num);
			
			off_num == 0 ? read_off(addr_tad, seg_num, dim_num) : read_mask(addr_tad, dim_num, seg_num, off_num);
			
			//DEBUG("Hi there 3\n");

		
		}
	}

	//DEBUG("read_cha_domen_out\n");

	return true;
}

void gh_hie::read_seg(unsigned int _addr_tad, unsigned int _dim_num)
{
	unsigned int cha_dim_num;
	unsigned int all_dim_seg_qua = 0;
	fseek(fp_tad, _addr_tad, SEEK_SET);
	fread(& cha_dim_num, 1, 1, fp_tad);
	cha_dim_num &= 0xff;
	cha_dim_num -= 1;

	fread(& seg_qua[cha_dim_num], 1, 1, fp_tad);
	seg_qua[cha_dim_num] &= 0xff;

	for(unsigned int i = 0; i <= cha_dim_num; i++) all_dim_seg_qua += seg_qua[i];
	if(all_dim_seg_qua > 254)
	{
		seg_qua[cha_dim_num] -= all_dim_seg_qua - 254;
		if(seg_qua[cha_dim_num] < 0) seg_qua[cha_dim_num] = 0;
	}

	
	seg[cha_dim_num] = new char * [seg_qua[cha_dim_num]];
	off[cha_dim_num] = new char ** [seg_qua[cha_dim_num]];
	masks[cha_dim_num] = new unsigned int ** [seg_qua[cha_dim_num]];
	off_qua[cha_dim_num] = new unsigned int[seg_qua[cha_dim_num]];
	unsigned int char_length;
	char buff[64];
	for(unsigned int i(0); i < seg_qua[cha_dim_num]; i++)
	{
		fread(& char_length, 1, 1, fp_tad);
		char_length &= 0x3f;
		seg[cha_dim_num][i] = new char[char_length + 1];
		fread(& buff, char_length, 1, fp_tad);
		buff[char_length] = '\0';
		strcpy(seg[cha_dim_num][i], buff);
	}

	DEBUG("dim: %d, all_dim: %d seg_qua: %d %d %d %d\n", cha_dim_num, all_dim_seg_qua, seg_qua[0], seg_qua[1], seg_qua[2], seg_qua[3]);
}

void gh_hie::read_off(unsigned int _addr_tad, unsigned int _seg_num, unsigned int _dim_num)
{
	fseek(fp_tad, _addr_tad, SEEK_SET);
	fread(& off_qua[_dim_num][_seg_num], 2, 1, fp_tad);
	off_qua[_dim_num][_seg_num] &= 0xffff;
	if(broken_item_cnt == 0)
	{
		off[_dim_num][_seg_num] = new char * [off_qua[_dim_num][_seg_num]];
		masks[_dim_num][_seg_num] = new unsigned int * [off_qua[_dim_num][_seg_num]];
	}
	unsigned int char_length = 0;
	char buff[gh::lng_size + 1];
	for(unsigned int i = broken_item_cnt; i < off_qua[_dim_num][_seg_num]; i++)
	{
		fread(& char_length, 1, 1, fp_tad);
		if(char_length == 0xfa)
		{
			broken_item_cnt = i;
			return;
		}
		char_length &= 0x3f;
		off[_dim_num][_seg_num][i] = new char [char_length + 1];
		fread(& buff, char_length, 1, fp_tad);
		buff[char_length] = '\0';	//	FIXME
		strcpy(off[_dim_num][_seg_num][i], buff);
	}
	broken_item_cnt = 0;
}

void gh_hie::read_mask(int _addr_tad, int _dim_num, int _seg_num, int _off_num)
{
	//DEBUG("%12d%12d%12d%12d\n", _addr_tad, _dim_num, _seg_num, _off_num);
	
	
	
	_off_num--;
	fseek(fp_tad, _addr_tad, SEEK_SET);
	fseek(fp_tad, -2, SEEK_CUR);
	fread(& mask_size_in_bytes[_dim_num], 2, 1, fp_tad);
	mask_size_in_bytes[_dim_num] &= 0xffff;
	mask_size_in_words[_dim_num] = (mask_size_in_bytes[_dim_num] + gh::word_size) / gh::word_size;
	masks[_dim_num][_seg_num][_off_num] = new unsigned int [mask_size_in_words[_dim_num]];
	fseek(fp_tad, 1, SEEK_CUR);
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
		fread(& masks[_dim_num][_seg_num][_off_num][i], gh::word_size, 1, fp_tad);
	
	//DEBUG("read_mask_out\n");
}

bool gh_hie::check_coo(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num)
{
	if(_dim_num >= gh::dim_qua) return false; // FIXME Log
	if(_seg_num >= seg_qua[_dim_num]) return false;
	if(_off_num >= off_qua[_dim_num][_seg_num]) return false;
	return true;
}

bool gh_hie::check_coo(unsigned int _dim_num, unsigned int _seg_num)
{
	if(_dim_num >= gh::dim_qua) return false; // FIXME Log
	if(_seg_num >= seg_qua[_dim_num]) return false;
	return true;
}

bool gh_hie::push(unsigned int _dim_num, unsigned int * _mask)
{
	if(sp[_dim_num] == gh::stack_size) return false;
	if(_mask == 0) return false;
	stack[_dim_num][sp[_dim_num]] = _mask;
	sp[_dim_num]++;
	return true;
}

bool gh_hie::push(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num)
{
	if(!check_coo(_dim_num, _seg_num, _off_num)) return false;
	return push(_dim_num, masks[_dim_num][_seg_num][_off_num]);
}

bool gh_hie::push(unsigned int _dim_num, const char * _seg_name, const char * _off_name)
{
	unsigned int seg_num;
	unsigned int off_num;
	for(seg_num = 0; seg_num < seg_qua[_dim_num]; seg_num++)
		if(!strcmp(seg[_dim_num][seg_num], _seg_name)) break;
	if(seg_num == seg_qua[_dim_num]) return false;

	for(off_num = 0; off_num < off_qua[_dim_num][seg_num]; off_num++)
		if(!strcmp(off[_dim_num][seg_num][off_num], _off_name)) break;
	if(off_num == off_qua[_dim_num][seg_num]) return false;

	return push(_dim_num, seg_num, off_num);
}

bool gh_hie::add(unsigned int _dim_num)
{

	//DEBUG("add SP = %d\n", sp[_dim_num]);

	if(sp[_dim_num] < 2) return false;
	new_masks[_dim_num][sp_nm[_dim_num]] = new unsigned int[mask_size_in_words[_dim_num]];
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
		new_masks[_dim_num][sp_nm[_dim_num]][i] = stack[_dim_num][sp[_dim_num] - 2][i] | stack[_dim_num][sp[_dim_num] - 1][i];
	stack[_dim_num][sp[_dim_num] - 2] = new_masks[_dim_num][sp_nm[_dim_num]];
	sp[_dim_num]--;
	sp_nm[_dim_num]++;
	return true;
}

bool gh_hie::mul(unsigned int _dim_num)
{

	//DEBUG("mul SP = %d\n", sp[_dim_num]);

	if(sp[_dim_num] < 2) return false;
	new_masks[_dim_num][sp_nm[_dim_num]] = new unsigned int[mask_size_in_words[_dim_num]];
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
		new_masks[_dim_num][sp_nm[_dim_num]][i] = stack[_dim_num][sp[_dim_num] - 2][i] & stack[_dim_num][sp[_dim_num] - 1][i];
	stack[_dim_num][sp[_dim_num] - 2] = new_masks[_dim_num][sp_nm[_dim_num]];
	sp[_dim_num]--;
	sp_nm[_dim_num]++;
	return true;
}

bool gh_hie::dup(unsigned int _dim_num)
{

	//DEBUG("dup SP = %d\n", sp[_dim_num]);

	if(sp[_dim_num] < 1) return false;
	if(sp[_dim_num] == gh::stack_size) return false;
	new_masks[_dim_num][sp_nm[_dim_num]] = new unsigned int[mask_size_in_words[_dim_num]];
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
		new_masks[_dim_num][sp_nm[_dim_num]][i] = stack[_dim_num][sp[_dim_num] - 1][i];
	stack[_dim_num][sp[_dim_num]] = new_masks[_dim_num][sp_nm[_dim_num]];
	sp[_dim_num]++;
	sp_nm[_dim_num]++;
	return true;
}

bool gh_hie::inv(unsigned int _dim_num)
{

	//DEBUG("inv SP = %d\n", sp[_dim_num]);

	if(sp[_dim_num] < 1) return false;
	new_masks[_dim_num][sp_nm[_dim_num]] = new unsigned int[mask_size_in_words[_dim_num]];
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
		new_masks[_dim_num][sp_nm[_dim_num]][i] = ~stack[_dim_num][sp[_dim_num] - 1][i];
	stack[_dim_num][sp[_dim_num] - 1] = new_masks[_dim_num][sp_nm[_dim_num]];
	sp_nm[_dim_num]++;
	return true;
}

unsigned int * gh_hie::pop(unsigned int _dim_num)
{

	//DEBUG("_pop SP = %d\n", sp[_dim_num]);

	if(sp[_dim_num] == 0) return 0;
	unsigned int * tmp = parse_mask(_dim_num, stack[_dim_num][--sp[_dim_num]]);
	if(sp[_dim_num] == 0)
	{
		for(unsigned int i(0); i < sp_nm[_dim_num]; i++) delete [] new_masks[_dim_num][i];
		sp_nm[_dim_num] = 0;
	}
	return tmp;
}

void gh_hie::coo_to_hie(unsigned int _dim_num, unsigned int _coo)
{
	coo::coo = _coo;
	coo::word = (unsigned int)(coo::coo / gh::word_size_in_bits);
	coo::bit = coo::coo - coo::word * gh::word_size_in_bits;
	coo::swap = gh::swap_bit[coo::bit];

	// FIXME to check
	for(coo::level = 0; coo::level < off_qua[_dim_num][0]; coo::level++)
		if(masks[_dim_num][0][coo::level][coo::word] & coo::swap) break;
	coo::level++;

	// FIXME to check
	for(coo::offset = 0; coo::offset < off_qua[_dim_num][coo::level]; coo::offset++)
		if(masks[_dim_num][coo::level][coo::offset][coo::word] & coo::swap) break;
}

unsigned int * gh_hie::get_children_from_level(unsigned int _dim_num, unsigned int _coo, unsigned int _target_level)
{
	//DEBUG("gh_hie: 1 _dim_num: %d, _coo: %d, _target_level: %d \n", _dim_num, _coo, _target_level);

	coo_to_hie(_dim_num, _coo);
	unsigned int coo_level = coo::level;
	unsigned int * array = parse_mask(_dim_num, masks[_dim_num][coo_level][coo::offset]);

	unsigned int * out = new unsigned int[array[0] + 1];
	out[0] = 1;
	for(unsigned int i(2); i <= array[0]; i++)
	{
		coo_to_hie(_dim_num, array[i]);
		if(coo::coo <= _coo) continue;
		if(coo::level == coo_level) break;
		if(coo::level == _target_level) out[out[0]++] = array[i];
	}
	delete [] array;
	return out;
}

unsigned int * gh_hie::get_children_from_level_2(unsigned int _dim_num, unsigned int _coo, unsigned int _target_level)
{
	coo_to_hie(_dim_num, _coo);
	push(_dim_num, coo::level, coo::offset);
	unsigned int * parents = get_parents(_dim_num, _coo, true);
	for(unsigned int i = 1; i < parents[0]; i++)
	{
		push(_dim_num, i, parents[i]);
		mul(_dim_num);
	}
	push(_dim_num, 0, _target_level - 1);
	mul(_dim_num);
	unsigned int * out = pop(_dim_num);
	return out;
}


unsigned int * gh_hie::get_children_from_level_1(unsigned int _dim_num, unsigned int _item_num, unsigned int _level)
{
	coo_to_hie(_dim_num, _item_num);
	unsigned int * array = parse_mask(_dim_num, masks[_dim_num][coo::level][coo::offset]);

	unsigned int next_num = gh::wrong_number;
	unsigned int * lev_arr = parse_mask(_dim_num, masks[_dim_num][0][coo::level -1]);
	for(unsigned int i(1); i <= lev_arr[0]; i++)
	{
		if(lev_arr[i] > _item_num)
		{
			next_num = lev_arr[i];
			break;
		}
	}

	unsigned int * out = new unsigned int[array[0] + 1];
	out[0] = 1;
	for(unsigned int i(1); i <= array[0]; i++)
	{
		if(array[i] > _item_num && array[i] < next_num) 
		{
			coo_to_hie(_dim_num, array[i]);
			if(coo::level == _level) out[out[0]++] = array[i];
		}
	}
	delete [] array;
	delete [] lev_arr;
	return out;
}



unsigned int * gh_hie::get_children_from_level_3(unsigned int _dim_num, unsigned int _coo, unsigned int _target_level)
{
	bind_coo(_dim_num, _coo);
	
	//DEBUG("\n_target_level=%d, hie=%d, seg=%d, hie_seg=%d, off=%d, segs_hie=%d\n\n",
	//		_target_level, coo_bind::hie, coo_bind::seg, coo_bind::hie_seg, coo_bind::off, coo_bind::segs_hie);
	

	//DEBUG("get 3 SP = %d target level = %d\n", sp[_dim_num], _target_level);

	if(!push(_dim_num, coo_bind::hie, _target_level)) return 0;

	//DEBUG("Again and again...1\n");

	if(!push(_dim_num, coo_bind::seg, coo_bind::off))
	{
		pop(_dim_num);
		return 0;
	}

	//DEBUG("Again and again...2\n");

	if(!mul(_dim_num)) return 0;

	//DEBUG("Again and again...3\n");

	unsigned int * items = pop(_dim_num);

	if(!push(_dim_num, coo_bind::hie, coo_bind::hie_seg)) return 0;

	//DEBUG("Again and again...4\n");

	if(!push(_dim_num, coo_bind::seg, coo_bind::off))
	{
		pop(_dim_num);
		return 0;
	}

	//DEBUG("Again and again...5\n");

	if(!mul(_dim_num)) return 0;

	//DEBUG("Again and again...6\n");

	unsigned int * sisters = pop(_dim_num);
	if(sisters[0] == 1)
	{
		delete [] sisters;
		return items;
	}
	unsigned int next_sister = - 1;
	for(unsigned int i = 1; i <= sisters[0]; i++)
	{
		if(sisters[i] > _coo)
		{
			next_sister = sisters[i];
			break;
		}
	}
	unsigned int * out = new unsigned int[items[0]];
	out[0] = 0;
	for(unsigned int i = 1; i <= items[0]; i++)
		if(items[i] > _coo && items[i] < next_sister) out[++out[0]] = items[i];

	delete [] sisters;
	delete [] items;

	return out;
}


void gh_hie::bind_coo(unsigned int _dim_num, unsigned int _coo)
{
	coo_bind::coo = _coo;
	coo_bind::word = (unsigned int)(coo_bind::coo / gh::word_size_in_bits);
	coo_bind::bit = coo_bind::coo - coo_bind::word * gh::word_size_in_bits;
	coo_bind::swap = gh::swap_bit[coo_bind::bit];

	for(coo_bind::hie = 0; coo_bind::hie < off_qua[_dim_num][1]; coo_bind::hie++)
		if(masks[_dim_num][1][coo_bind::hie][coo_bind::word] & coo_bind::swap) break;
	coo_bind::hie += 2;

	for(coo_bind::hie_seg = 0; coo_bind::hie_seg < off_qua[_dim_num][coo_bind::hie]; coo_bind::hie_seg++)
		if(masks[_dim_num][coo_bind::hie][coo_bind::hie_seg][coo_bind::word] & coo_bind::swap) break;

//	for(coo_bind::seg = coo_bind::hie + off_qua[_dim_num][1]; coo_bind::seg < seg_qua[_dim_num]; coo_bind::seg++)
//		if(!strcmp(seg[_dim_num][coo_bind::seg], off[_dim_num][coo_bind::hie][coo_bind::hie_seg])) break;

	for(coo_bind::seg = 0; coo_bind::seg < seg_qua[_dim_num]; coo_bind::seg++)
		if(!strcmp(seg[_dim_num][coo_bind::seg], off[_dim_num][coo_bind::hie][coo_bind::hie_seg])) break;

	for(coo_bind::off = 0; coo_bind::off < off_qua[_dim_num][coo_bind::seg]; coo_bind::off++)
		if(masks[_dim_num][coo_bind::seg][coo_bind::off][coo_bind::word] & coo_bind::swap) break;

}

unsigned int gh_hie::get_coo_level_3(unsigned int _dim_num, unsigned int _coo, unsigned int & _hie, unsigned int & _level)
{
	coo_bind::coo = _coo;
	coo_bind::word = (unsigned int)(coo_bind::coo / gh::word_size_in_bits);
	coo_bind::bit = coo_bind::coo - coo_bind::word * gh::word_size_in_bits;
	coo_bind::swap = gh::swap_bit[coo_bind::bit];

	for(coo_bind::hie = 0; coo_bind::hie < off_qua[_dim_num][1]; coo_bind::hie++)
		if(masks[_dim_num][1][coo_bind::hie][coo_bind::word] & coo_bind::swap) break;
	coo_bind::hie += 2;

	for(coo_bind::hie_seg = 0; coo_bind::hie_seg < off_qua[_dim_num][coo_bind::hie]; coo_bind::hie_seg++)
		if(masks[_dim_num][coo_bind::hie][coo_bind::hie_seg][coo_bind::word] & coo_bind::swap) break;

	for(coo_bind::seg = coo_bind::hie + off_qua[_dim_num][1]; coo_bind::seg < seg_qua[_dim_num]; coo_bind::seg++)
		if(!strcmp(seg[_dim_num][coo_bind::seg], off[_dim_num][coo_bind::hie][coo_bind::hie_seg])) break;

	for(coo_bind::off = 0; coo_bind::off < off_qua[_dim_num][coo_bind::seg]; coo_bind::off++)
		if(masks[_dim_num][coo_bind::seg][coo_bind::off][coo_bind::word] & coo_bind::swap) break;
	_hie = coo_bind::hie;
	_level = coo_bind::hie_seg;

	return coo_bind::hie_seg;
}

unsigned int * gh_hie::get_all_children(unsigned int _dim_num, unsigned int _coo)
{
	coo_to_hie(_dim_num, _coo);
	return parse_mask(_dim_num, masks[_dim_num][coo::level][coo::offset]);
}

unsigned int * gh_hie::parse_mask(unsigned int _dim_num, unsigned int * _mask)
{
	unsigned int * array =  new unsigned int[mask_size_in_bytes[_dim_num] * gh::bits_in_byte];
	array[0] = 1;
	unsigned int item = 0;
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
	{
		if(_mask[i] > 0)
		{
			for(unsigned int j(0); j < gh::word_size_in_bits && item < dim_size[_dim_num]; j++, item++)
				if((gh::swap_bit[j] & _mask[i]) > 0) array[array[0]++] = item;
		}
		else item += gh::word_size_in_bits;
	}
	array[0]--;
	return array;
}

unsigned int gh_hie::first_item_from_mask(unsigned int _dim_num, unsigned int _seg, unsigned int _off)
{
	unsigned int item = 0;
	for(unsigned int i(0); i < mask_size_in_words[_dim_num]; i++)
	{
		if(masks[_dim_num][_seg][_off][i] > 0)
		{
			for(unsigned int j(0); j < gh::word_size_in_bits && item < dim_size[_dim_num]; j++, item++)
				if((gh::swap_bit[j] & masks[_dim_num][_seg][_off][i]) > 0) return item;
		}
		else item += gh::word_size_in_bits;
	}
	return item;
}

unsigned int * gh_hie::get_parents(unsigned int _dim_num, unsigned int _coo, bool _is_addr)
{
	coo_to_hie(_dim_num, _coo);
	unsigned int * parents = new unsigned int[gh::levels_max];
	parents[0] = coo::level;
	for(unsigned int i(1); i < coo::level; i++)
	{
		for(unsigned int j(0); j < off_qua[_dim_num][i]; j++)
		{
			if(masks[_dim_num][i][j][coo::word] & coo::swap)
			{
				parents[i] = _is_addr ? j : first_item_from_mask(_dim_num, i, j);
				break;
			}
		}
	}
	return parents;
}


char ** gh_hie::get_parents_names(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
{
	coo_to_hie(_dim_num, _coo);
	_size = off_qua[_dim_num][0] > coo::level ? coo::level : seg_qua[_dim_num];

	char ** parents = new char * [_size];
	for(unsigned int i(0); i < _size; i++)
	{
		parents[i] = new char[gh::lng_size + 1];
		for(unsigned int j(0); j < off_qua[_dim_num][i]; j++)
		{
			if(masks[_dim_num][i][j][coo::word] & coo::swap)
			{
				strcpy(parents[i], off[_dim_num][i][j]);
				break;
			}
			else parents[i][0] = '\0';
		}
	}
	return parents;
}

char ** gh_hie::get_parents_names_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
{
	coo_to_hie(_dim_num, _coo);
	_size = off_qua[_dim_num][0] > coo::level ? coo::level + 1 : seg_qua[_dim_num];

	char ** parents = new char * [_size];
	for(unsigned int i(0); i < _size; i++)
	{
		parents[i] = new char[gh::lng_size + 1];
		for(unsigned int j(0); j < off_qua[_dim_num][i]; j++)
		{
			if(masks[_dim_num][i][j][coo::word] & coo::swap)
			{
				strcpy(parents[i], off[_dim_num][i][j]);
				break;
			}
			else parents[i][0] = '\0';
		}
	}
	return parents;
}

unsigned int gh_hie::get_coo_level(unsigned int _dim_num, unsigned int _coo)
{
	coo_to_hie(_dim_num, _coo);
	return coo::level;
}


char ** gh_hie::get_info(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
{
	coo_bind::coo = _coo;
	coo_bind::word = (unsigned int)(coo_bind::coo / gh::word_size_in_bits);
	coo_bind::bit = coo_bind::coo - coo_bind::word * gh::word_size_in_bits;
	coo_bind::swap = gh::swap_bit[coo_bind::bit];

	for(coo_bind::hie = 0; coo_bind::hie < off_qua[_dim_num][1]; coo_bind::hie++)
		if(masks[_dim_num][1][coo_bind::hie][coo_bind::word] & coo_bind::swap) break;
	coo_bind::hie += 2;

	for(coo_bind::hie_seg = 0; coo_bind::hie_seg < off_qua[_dim_num][coo_bind::hie]; coo_bind::hie_seg++)
		if(masks[_dim_num][coo_bind::hie][coo_bind::hie_seg][coo_bind::word] & coo_bind::swap) break;

	for(coo_bind::seg = coo_bind::hie + off_qua[_dim_num][1]; coo_bind::seg < seg_qua[_dim_num]; coo_bind::seg++)
		if(!strcmp(seg[_dim_num][coo_bind::seg], off[_dim_num][coo_bind::hie][coo_bind::hie_seg])) break;

	for(coo_bind::off = 0; coo_bind::off < off_qua[_dim_num][coo_bind::seg]; coo_bind::off++)
		if(masks[_dim_num][coo_bind::seg][coo_bind::off][coo_bind::word] & coo_bind::swap) break;

	//DEBUG("Hi 06: hie_seg: %d, seg: %d, off: %d\n", coo_bind::hie_seg, coo_bind::seg, coo_bind::off);

	unsigned int * hie_segs = gh_hie::get_hie_segs(_dim_num, coo_bind::hie);

	//DEBUG("\nhie_segs\n");

	
	
	
	
	/////////////////////////////////////////////////////////////////////////////////
	//unsigned int hie_size = off_qua[_dim_num][coo_bind::hie];

	//unsigned int * a = core->get_hie_segs(dim_num, hie_num);
	//for(unsigned i = 0; i < hie_size; i++)
	//{
	//	printf("%d\n", hie_segs[i]);
	//}
	//DEBUG("\n");

	/////////////////////////////////////////////////////////////////////////////////
	

	
	
	
	
	
	
	char ** parents = new char * [off_qua[_dim_num][coo_bind::hie]];

	//DEBUG("parents\n");

	_size = 0;
	for(unsigned int i = 0; i < off_qua[_dim_num][coo_bind::hie]; i++)
	{
		for(unsigned int j = 0; j < off_qua[_dim_num][hie_segs[i]]; j++)
		{
			if(masks[_dim_num][hie_segs[i]][j][coo_bind::word] & coo_bind::swap)
			{
				//DEBUG("\tpar 01\n");
				parents[i] = new char[gh::lng_size + 1];
				//DEBUG("\tpar 02\n");
				strcpy(parents[i], off[_dim_num][hie_segs[i]][j]); 
				//DEBUG("parents: %s\n", parents[i]);
				//DEBUG("\tpar 03\n");
				_size++;
				//DEBUG("\tpar 04\n");
				break;
			}
		}
	}

	//DEBUG("Hi 06\n");

	delete [] hie_segs;

	//DEBUG("Hi 07\n");

	return parents;
}

char ** gh_hie::get_info_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size, unsigned int & _hie)
{
	//DEBUG("Hi there...\n");
	
	coo_bind::coo = _coo;
	coo_bind::word = (unsigned int)(coo_bind::coo / gh::word_size_in_bits);
	coo_bind::bit = coo_bind::coo - coo_bind::word * gh::word_size_in_bits;
	coo_bind::swap = gh::swap_bit[coo_bind::bit];

	for(coo_bind::hie = 0; coo_bind::hie < off_qua[_dim_num][1]; coo_bind::hie++)
		if(masks[_dim_num][1][coo_bind::hie][coo_bind::word] & coo_bind::swap) break;
	coo_bind::hie += 2;

	for(coo_bind::hie_seg = 0; coo_bind::hie_seg < off_qua[_dim_num][coo_bind::hie]; coo_bind::hie_seg++)
		if(masks[_dim_num][coo_bind::hie][coo_bind::hie_seg][coo_bind::word] & coo_bind::swap) break;

	for(coo_bind::seg = coo_bind::hie + off_qua[_dim_num][1]; coo_bind::seg < seg_qua[_dim_num]; coo_bind::seg++)
		if(!strcmp(seg[_dim_num][coo_bind::seg], off[_dim_num][coo_bind::hie][coo_bind::hie_seg])) break;

	for(coo_bind::off = 0; coo_bind::off < off_qua[_dim_num][coo_bind::seg]; coo_bind::off++)
		if(masks[_dim_num][coo_bind::seg][coo_bind::off][coo_bind::word] & coo_bind::swap) break;

	unsigned int * hie_segs = gh_hie::get_hie_segs(_dim_num, coo_bind::hie);
	char ** parents = new char * [off_qua[_dim_num][coo_bind::hie]];
	_size = 0;
	for(unsigned int i = 0; i < off_qua[_dim_num][coo_bind::hie]; i++)
	{
		for(unsigned int j = 0; j < off_qua[_dim_num][hie_segs[i]]; j++)
		{
			if(masks[_dim_num][hie_segs[i]][j][coo_bind::word] & coo_bind::swap)
			{
				parents[i] = new char[gh::lng_size + 1];
				strcpy(parents[i], off[_dim_num][hie_segs[i]][j]); 
				_size++;
				break;
			}
		}
	}
	delete [] hie_segs;
	_hie = coo_bind::hie;
	
	//DEBUG("~~~~~~~~~~~~~ hie:%d\n", _hie);
	
	return parents;
}





unsigned int * gh_hie::get_hie_segs(unsigned int _dim_num, unsigned int _hie_num)
{
	unsigned int hie_size = off_qua[_dim_num][_hie_num];
	char tmp[100];
	strcpy(tmp, off[_dim_num][_hie_num][0]);

	char * a = strstr(off[_dim_num][_hie_num][0], " level 0");
	if(a) * a = '\0';

	unsigned int * hie_segs = new unsigned int[hie_size];
	// Бывают такие CIP базы данных, в которых нестандартным образом называется первая строчка в иерархиях(на конце нет " level 0").
	// Из-за этого приходится первому сегменту присваивать название вручную.
	
	hie_segs[0] = _hie_num; 
	for(unsigned int i = 1; i < hie_size; i++)
	{
		for(unsigned int j = 0; j < seg_qua[_dim_num]; j++) if(!strcmp(off[_dim_num][_hie_num][i], seg[_dim_num][j])) hie_segs[i] = j;
	}
	strcpy(off[_dim_num][_hie_num][0], tmp);

	return hie_segs;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void gh_hie::log()
{
	FILE * fp = fopen("./hie_orig.log", "w");
	if(!fp)
	{
		ERROR;
		printf("gh_core:Cannot find the \"./rep\" directory\n");
		return;
	}
	for(unsigned int i(0); i < gh::dim_qua; i++)
	{
		fprintf(fp, "********************** Dimension:%d **********************\n", i);
		for(unsigned int j(0); j < seg_qua[i]; j++)
		{
			fprintf(fp, "Segment: %3d %s\n", j, seg[i][j]);
			for(unsigned int k(0); k < off_qua[i][j]; k++)
			{
				fprintf(fp, "\tOffset: %5d :%-82s\n", k, off[i][j][k]);

// 				for(int l = 0; l < mask_size_in_words[i]; l++)
// 				{
// 					unsigned int segm = l * gh::word_size * 8;
// 					if(masks[i][j][k][l] > 0)
// 					{
// 						unsigned int mask_offset = segm;
// 						for(unsigned int a_bit = 0x80; (a_bit > 0) && (mask_offset < gh::dim_size[i]); a_bit >>= 1, mask_offset++)	//	1-й байт
// 							if((a_bit & masks[i][j][k][l]) > 0);
// 								//fprintf(fp, "\t\t\t\t\t\t\t%8d : %s\n", mask_offset, hed->get_attr_by_num(i, 1, mask_offset));
// 						for(unsigned int a_bit = 0x8000; (a_bit > 0x80) && (mask_offset < gh::dim_size[i]); a_bit >>= 1, mask_offset++)	//	2-й байт
// 							if((a_bit & masks[i][j][k][l]) > 0);
// 								//fprintf(fp, "\t\t\t\t\t\t\t%8d : %s\n", mask_offset, hed->get_attr_by_num(i, 1, mask_offset));
// 						for(unsigned int a_bit = 0x800000; (a_bit > 0x8000) && (mask_offset < gh::dim_size[i]); a_bit >>= 1, mask_offset++)	//	3-й байт
// 							if((a_bit & masks[i][j][k][l]) > 0);
// 								//fprintf(fp, "\t\t\t\t\t\t\t%8d : %s\n", mask_offset, hed->get_attr_by_num(i, 1, mask_offset));
// 						for(unsigned int a_bit = 0x80000000; (a_bit > 0x800000) && (mask_offset < gh::dim_size[i]); a_bit >>= 1, mask_offset++)	//	4-й байт
// 							if((a_bit & masks[i][j][k][l]) > 0);
// 								//fprintf(fp, "\t\t\t\t\t\t\t%8d : %s\n", mask_offset, hed->get_attr_by_num(i, 1, mask_offset));
// 					}
// 				}


			}
		}
	}
	fclose(fp);
}

