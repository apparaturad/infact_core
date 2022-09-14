#ifndef GH_CORE_H
#define GH_CORE_H

#define DEBUG printf
#define ERROR printf("\n\t*************\n\t*   error   *\n\t*************\n\n");

#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>

namespace gh
{
	const unsigned int index_max = 100000;
	const unsigned int wrong_number = 0xffffffff;
	const unsigned int dim_qua = 4;
	const unsigned int per_max = 500;
	const unsigned int lng_size = 80;
	const unsigned int sht_size = 40;
	const unsigned int tag_size = 40;
	const unsigned int stack_size = 1000;
	const unsigned int levels_max = 300;
	const unsigned int bits_in_byte = 8;
	const unsigned int word_size = sizeof(unsigned int);
	const unsigned int word_size_in_bits = word_size * bits_in_byte;
	const unsigned int idx_start_point = 0x200;
	const unsigned int swap_bit[] =
	{
		0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1,
		0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
		0x800000, 0x400000, 0x200000, 0x100000, 0x80000, 0x40000, 0x20000, 0x10000,
		0x80000000, 0x40000000, 0x20000000, 0x10000000, 0x8000000, 0x4000000, 0x2000000, 0x1000000
	};
}

class gh_hed
{
	friend class gh_core;
	gh_hed(FILE *);
	~gh_hed();

	unsigned int size[gh::dim_qua];
	char **** att;
	unsigned int get(unsigned int, const char *);
	unsigned int get(unsigned int, unsigned int);
	void log(const char *);
};

class gh_idx
{
	friend class gh_core;
	gh_idx(FILE *, unsigned int * _dim_size);
	gh_idx(FILE *);
	~gh_idx();

	FILE * fp;
	unsigned int inf_addr_size;
	unsigned int * dim_size;
	unsigned long *** array;
	void reverse(unsigned short &);
	int get_dom_data(unsigned short, unsigned short, unsigned short, unsigned long &);
	unsigned long get(unsigned short _mkt, unsigned short _prd, unsigned short _fct);
	void idx_to_array();
};

class gh_inf
{
	friend class gh_core;
	gh_inf(FILE *);
	~gh_inf();

	FILE * fp;
	double * digs;
	void fill_nulls(unsigned short);
	void read_digs(unsigned short, unsigned short, unsigned short, bool);
	double * get(unsigned long, unsigned short);
};

class gh_hie
{
	friend class gh_core;
	FILE * fp_chr, * fp_tad;

	//unsigned int seg_num;
	void log();
	void run_cha_domens();
	bool read_cha_domen(unsigned int);
	void read_seg(unsigned int _addr_tad, unsigned int _seg_dim);
	void read_off(unsigned int _addr_tad, unsigned int _seg_num, unsigned int _dim_num);
	void read_mask(int _addr_tad, int _dim_num, int _seg_num, int _off_num);
	unsigned int dim_num;
	unsigned int dim_shift;
	int broken_item_cnt;
	unsigned int mask_size_in_bytes[gh::dim_qua];

	unsigned int * passed;

	unsigned int *** stack;
	unsigned int sp[gh::dim_qua];
	unsigned int *** new_masks;
	unsigned int sp_nm[gh::dim_qua];

	unsigned int * parse_mask(unsigned int _dim_num, unsigned int * _mask);
	bool check_coo(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num);
	bool check_coo(unsigned int _dim_num, unsigned int _seg_num);

	void coo_to_hie(unsigned int _dim_num, unsigned int _item);
	unsigned int first_item_from_mask(unsigned int _dim_num, unsigned int _seg, unsigned int _off);


	gh_hie(FILE *, FILE *);
	~gh_hie();

	unsigned int dim_size[gh::dim_qua];

	bool push(unsigned int _dim_num, unsigned int * _mask);
	bool push(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num);
	bool push(unsigned int _dim_num, const char * _seg_name, const char * _off_name);

	unsigned int * pop(unsigned int _dim_num);
	bool pop1(unsigned int _dim_num, unsigned int * _array);

	bool add(unsigned int _dim_num);
	bool mul(unsigned int _dim_num);
	bool dup(unsigned int _dim_num);
	bool inv(unsigned int _dim_num);

	unsigned int * get_parents(unsigned int _dim_num, unsigned int _coo, bool _is_addr);
	unsigned int * get_all_children(unsigned int _dim_num, unsigned int _item);
	unsigned int * seg_qua;
	char *** seg;
	unsigned int ** off_qua;
	char **** off;

	unsigned int mask_size_in_words[gh::dim_qua];
	unsigned int * mask_qua;
	unsigned int **** masks;

	unsigned int * get_children_from_level(unsigned int _dim_num, unsigned int _item_num, unsigned int _level);
	unsigned int * get_children_from_level_1(unsigned int _dim_num, unsigned int _item_num, unsigned int _level);
	unsigned int * get_children_from_level_2(unsigned int _dim_num, unsigned int _item_num, unsigned int _level);
	unsigned int * get_children_from_level_3(unsigned int _dim_num, unsigned int _item_num, unsigned int _level);

	unsigned int get_coo_level(unsigned int _dim_num, unsigned int _coo);
	unsigned int get_coo_level_3(unsigned int _dim_num, unsigned int _coo, unsigned int & _hie, unsigned int & _level);
	char ** get_parents_names(unsigned int _dim_num, unsigned int _coo, unsigned int & _size);
	char ** get_parents_names_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size);
	char ** get_info(unsigned int _dim_num, unsigned int _coo, unsigned int & _size);
	char ** get_info_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size, unsigned int & _hie);
	void bind_coo(unsigned int _dim_num, unsigned int _coo);
	unsigned int * get_hie_segs(unsigned int _dim_num, unsigned int _hie_num);
};

class gh_core
{
	gh_hed * hed;
	gh_idx * idx;
	gh_inf * inf;
	gh_hie * hie;

	unsigned int conn_db(const char *, bool _is_opt);
	void hie_log(const char * _hed_name);
	unsigned int integrity;

	public:
		gh_core(const char *, const bool _is_opt, const bool _is_logs);
		~gh_core();

		inline const unsigned int get_integrity()
		{return integrity;}

		inline unsigned int get_dim_size(unsigned int _dim_num)
		{return _dim_num > gh::dim_qua ? 0 : hed->size[_dim_num];}

		inline bool push(unsigned int _dim_num, unsigned int * _mask)
		{return hie->push(_dim_num, _mask);}

		inline bool push(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num)
		{return hie->push(_dim_num, _seg_num, _off_num);}

		inline bool push(unsigned int _dim_num, const char * _seg_name, const char * _off_name)
		{return hie->push(_dim_num, _seg_name, _off_name);}

		inline bool add(unsigned int _dim_num)
		{return hie->add(_dim_num);}

		inline bool mul(unsigned int _dim_num)
		{return hie->mul(_dim_num);}

		inline bool dup(unsigned int _dim_num)
		{return hie->dup(_dim_num);}

		inline bool inv(unsigned int _dim_num)
		{return hie->inv(_dim_num);}

		inline unsigned int * pop(unsigned int _dim_num)
		{return hie->pop(_dim_num);}

		inline unsigned int **** get_masks()
		{return hie->masks;}

		inline unsigned int * get_mask_qua()
		{return hie->mask_qua;}

		inline unsigned int * get_mask_size_in_words()
		{return hie->mask_size_in_words;}
		
		
		///////////////////////////////////////////////////////////////////////////////////////
		
		
		inline unsigned int * get_parents(unsigned int _dim_num, unsigned int _coo, bool _is_addr)
		{return hie->get_parents(_dim_num, _coo, _is_addr);}

		inline unsigned int * get_all_children(unsigned int _dim_num, unsigned int _item)
		{return hie->get_all_children(_dim_num, _item);}

		inline unsigned int * get_children_from_level(unsigned int _dim_num, unsigned int _item_num, unsigned int _level)
		{return hie->get_children_from_level(_dim_num, _item_num, _level);}

		inline unsigned int * get_children_from_level_1(unsigned int _dim_num, unsigned int _item_num, unsigned int _level)
		{return hie->get_children_from_level_1(_dim_num, _item_num, _level);}

		inline unsigned int * get_children_from_level_2(unsigned int _dim_num, unsigned int _item_num, unsigned int _level)
		{return hie->get_children_from_level_2(_dim_num, _item_num, _level);}

		// Для CIP баз
		inline unsigned int * get_children_from_level_3(unsigned int _dim_num, unsigned int _item_num, unsigned int _level)
		{return hie->get_children_from_level_3(_dim_num, _item_num, _level);}


		
		
		
		
		
		inline unsigned int get_seg_qua(unsigned int _dim_num)
		{return hie->seg_qua[_dim_num];}

		inline char * get_seg_name(unsigned int _dim_num, unsigned int _seg_num)
		{return hie->seg[_dim_num][_seg_num];}

		inline unsigned int get_off_qua(unsigned int _dim_num, unsigned int _seg_num)
		{return hie->off_qua[_dim_num][_seg_num];}

		inline char * get_off_name(unsigned int _dim_num, unsigned int _seg_num, unsigned int _off_num)
		{return hie->off[_dim_num][_seg_num][_off_num];}

		inline unsigned int first_item_from_mask(unsigned int _dim_num, unsigned int _seg, unsigned int _off)
		{return hie->first_item_from_mask(_dim_num, _seg, _off);}

		inline unsigned int get_coo_level(unsigned int _dim_num, unsigned int _coo)
		{return hie->get_coo_level(_dim_num, _coo);}

		// Для CIP
		inline unsigned int get_coo_level_3(unsigned int _dim_num, unsigned int _coo, unsigned int & _hie, unsigned int & _level)
		{return hie->get_coo_level_3(_dim_num, _coo, _hie, _level);}

		inline char ** get_parents_names(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
		{return hie->get_parents_names(_dim_num, _coo, _size);}

		inline char ** get_parents_names_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
		{return hie->get_parents_names_1(_dim_num, _coo, _size);}

		inline char ** get_info(unsigned int _dim_num, unsigned int _coo, unsigned int & _size)
		{return hie->get_info(_dim_num, _coo, _size);};

		inline char ** get_info_1(unsigned int _dim_num, unsigned int _coo, unsigned int & _size, unsigned int & _hie)
		{return hie->get_info_1(_dim_num, _coo, _size, _hie);};


		// Временно для отладки

		inline unsigned int * get_hie_segs(unsigned int _dim_num, unsigned int _hie_num)
		{return hie->get_hie_segs(_dim_num, _hie_num);};



		template <typename T1, typename T2, typename T3, typename T4>
			double get(T1 _mkt, T2 _prd, T3 _fct, T4 _per)
			{
				unsigned int mkt = hed->get(0, _mkt);
				if(mkt == gh::wrong_number)
				{
					DEBUG("wrong market\n");
					return 0;
				}

				unsigned int prd = hed->get(1, _prd);
				if(prd == gh::wrong_number)
				{
					DEBUG("wrong product\n");
					return 0;
				}

				unsigned int fct = hed->get(2, _fct);
				if(fct == gh::wrong_number)
				{
					DEBUG("wrong fact\n");
					return 0;
				}

				unsigned int per = hed->get(3, _per);
				if(per == gh::wrong_number)
				{
					DEBUG("wrong period\n");
					return 0;
				}


				unsigned long addr = idx->get(mkt, prd, fct);
// 				if(addr == 0) return .0;
				if(addr == 0)
				{
					//DEBUG("wrong idx: mkt: %d, prd: %d, fct: %d\n", mkt, prd, fct);
					return 0;
				}

				double * dig = inf->get(addr, hed->size[3]);
				double fig = dig[per];
				delete [] dig;
				return fig;
			}

		template <typename T1, typename T2, typename T3>
			double * get(T1 _mkt, T2 _prd, T3 _fct)
			{
				unsigned int mkt = hed->get(0, _mkt);
				if(mkt == gh::wrong_number) return 0;

				unsigned int prd = hed->get(1, _prd);
				if(prd == gh::wrong_number) return 0;

				unsigned int fct = hed->get(2, _fct);
				if(fct == gh::wrong_number) return 0;

				unsigned long addr = idx->get(mkt, prd, fct);
				if(addr == 0)
				{
					double * fig = new double[gh::per_max];
					for(unsigned int i(0); i < gh::per_max; i++) fig[i] = .0;
					return fig;
				}

				double * dig = inf->get(addr, hed->size[3]);
				return dig;
			}

		// hed
		const char * get_attr_by_num(unsigned int _dim_num, unsigned int _att_num, unsigned int _item_num);

		inline unsigned int get_num_by_tag(int _dim_num, const char * _tag)
		{return hed->get(_dim_num, _tag);};
};

#endif
