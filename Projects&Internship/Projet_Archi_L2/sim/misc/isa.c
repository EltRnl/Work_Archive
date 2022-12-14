#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "isa.h"


/* Are we running in GUI mode? */
extern int gui_mode;

/* Bytes Per Line = Block size of memory */
#define BPL 32

struct {
    char *name;
    int id;
} reg_table[REG_NONE+1] = 
{
    {"%eax",   REG_EAX},
    {"%ecx",   REG_ECX},
    {"%edx",   REG_EDX},
    {"%ebx",   REG_EBX},
    {"%esp",   REG_ESP},
    {"%ebp",   REG_EBP},
    {"%esi",   REG_ESI},
    {"%edi",   REG_EDI},
    {"----",  REG_NONE}
};


reg_id_t find_register(char *name)
{
    int i;
    for (i = 0; i < REG_NONE; i++)
	if (!strcmp(name, reg_table[i].name))
	    return reg_table[i].id;
    return REG_NONE;
}

char *reg_name(reg_id_t id)
{
    if (id < REG_NONE)
	return reg_table[id].name;
    else
	return reg_table[REG_NONE].name;
}

instr_t instruction_set[] = 
{
    {"nop",    HPACK(I_NOP, 0), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"halt",   HPACK(I_HALT, 0), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"rrmovl", HPACK(I_RRMOVL, 0), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"irmovl", HPACK(I_IRMOVL, 0), 6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"rmmovl", HPACK(I_RMMOVL, 0), 6, R_ARG, 1, 1, M_ARG, 1, 0 },
    {"mrmovl", HPACK(I_MRMOVL, 0), 6, M_ARG, 1, 0, R_ARG, 1, 1 },
    {"addl",   HPACK(I_ALU, A_ADD), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"subl",   HPACK(I_ALU, A_SUB), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"andl",   HPACK(I_ALU, A_AND), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"xorl",   HPACK(I_ALU, A_XOR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* JB ajout sall et sarl (shift arithmetic left/right) */
    {"sall",   HPACK(I_ALU, A_SAL), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"sarl",   HPACK(I_ALU, A_SAR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"jmp",    HPACK(I_JXX, J_YES), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jle",    HPACK(I_JXX, J_LE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jl",     HPACK(I_JXX, J_L), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"je",     HPACK(I_JXX, J_E), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jne",    HPACK(I_JXX, J_NE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jge",    HPACK(I_JXX, J_GE), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"jg",     HPACK(I_JXX, J_G), 5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"call",   HPACK(I_CALL, 0),    5, I_ARG, 1, 4, NO_ARG, 0, 0 },
    {"ret",    HPACK(I_RET, 0), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"pushl",  HPACK(I_PUSHL, 0) , 2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"popl",   HPACK(I_POPL, 0) ,  2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    /* JB versions imm??diates de toutes les op??rations */
    {"iaddl",  HPACK(I_ALUI, A_ADD),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"isubl",  HPACK(I_ALUI, A_SUB),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"iandl",  HPACK(I_ALUI, A_AND),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"ixorl",  HPACK(I_ALUI, A_XOR),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"isall",  HPACK(I_ALUI, A_SAL),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"isarl",  HPACK(I_ALUI, A_SAR),  6, I_ARG, 2, 4, R_ARG, 1, 0 },
    {"leave",  HPACK(I_LEAVE, 0), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    /* this is just a hack to make the I_POP2 code have an associated name */
    {"pop2",   HPACK(I_POP2, 0) , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    /* JB ajout 'jreg' et 'jmem' */
    {"jreg",   HPACK(I_JREG, 0) , 2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"jmem",   HPACK(I_JMEM, 0), 6, M_ARG, 1, 0, NO_ARG, 0, 0 },

    /* For allocation instructions, arg1hi indicates number of bytes */
    {".byte",  0x00, 1, I_ARG, 0, 1, NO_ARG, 0, 0 },
    {".word",  0x00, 2, I_ARG, 0, 2, NO_ARG, 0, 0 },
    {".long",  0x00, 4, I_ARG, 0, 4, NO_ARG, 0, 0 },
    {NULL,     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 }
};

instr_t invalid_instr =
    {"XXX",     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 };

instr_ptr find_instr(char *name)
{
    int i;
    for (i = 0; instruction_set[i].name; i++)
	if (strcmp(instruction_set[i].name,name) == 0)
	    return &instruction_set[i];
    return NULL;
}

/* Return name of instruction given its encoding */
char *iname(int instr) {
    int i;
    for (i = 0; instruction_set[i].name; i++) {
	if (instr == instruction_set[i].code)
	    return instruction_set[i].name;
    }
    return "<bad>";
}


instr_ptr bad_instr()
{
    return &invalid_instr;
}


mem_t init_mem(int len)
{

    mem_t result = (mem_t) malloc(sizeof(mem_rec));
    len = ((len+BPL-1)/BPL)*BPL;
    result->len = len;
    result->contents = (byte_t *) calloc(len, 1);
    return result;
}

void clear_mem(mem_t m)
{
    memset(m->contents, 0, m->len);
}

void free_mem(mem_t m)
{
    free((void *) m->contents);
    free((void *) m);
}

mem_t copy_mem(mem_t oldm)
{
    mem_t newm = init_mem(oldm->len);
    memcpy(newm->contents, oldm->contents, oldm->len);
    return newm;
}

bool_t diff_mem(mem_t oldm, mem_t newm, FILE *outfile)
{
    word_t pos;
    int len = oldm->len;
    bool_t diff = FALSE;
    if (newm->len < len)
	len = newm->len;
    for (pos = 0; (!diff || outfile) && pos < len; pos += 4) {
	word_t ov, nv;
	get_word_val(oldm, pos, &ov);
	get_word_val(newm, pos, &nv);
	if (nv != ov) {
	    diff = TRUE;
	    if (outfile)
		fprintf(outfile, "0x%.4x:\t0x%.8x\t0x%.8x\n", pos, ov, nv);
	}
    }
    return diff;
}

int hex2dig(char c)
{
    if (isdigit((int)c))
	return c - '0';
    if (isupper((int)c))
	return c - 'A' + 10;
    else
	return c - 'a' + 10;
}

#define LINELEN 4096
int load_mem(mem_t m, FILE *infile, int report_error)
{
    /* Read contents of .yo file */
    char buf[LINELEN];
    char c, ch, cl;
    int byte_cnt = 0;
    int lineno = 0;
    word_t bytepos = 0;
#ifdef HAS_GUI
    int empty_line = 1;
    int addr = 0;
    char hexcode[15];

    /* For display */
    int line_no = 0;
    char line[LINELEN];

    int index = 0;
#endif /* HAS_GUI */   

    while (fgets(buf, LINELEN, infile)) {
	int cpos = 0;
#ifdef HAS_GUI
	empty_line = 1;
#endif
	lineno++;
	/* Skip white space */
	while (isspace((int)buf[cpos]))
	    cpos++;

	if (buf[cpos] != '0' ||
	    (buf[cpos+1] != 'x' && buf[cpos+1] != 'X'))
	    continue; /* Skip this line */      
	cpos+=2;

	/* Get address */
	bytepos = 0;
	while (isxdigit((int)(c=buf[cpos]))) {
	    cpos++;
	    bytepos = bytepos*16 + hex2dig(c);
	}

	while (isspace((int)buf[cpos]))
	    cpos++;

	if (buf[cpos++] != ':') {
	    if (report_error) {
		fprintf(stderr, "Error reading file. Expected colon\n");
		fprintf(stderr, "Line %d:%s\n", lineno, buf);
		fprintf(stderr,
			"Reading '%c' at position %d\n", buf[cpos], cpos);
	    }
	    return 0;
	}

#ifdef HAS_GUI
	addr = bytepos;
#endif

	while (isspace((int)buf[cpos]))
	    cpos++;

#ifdef HAS_GUI
	index = 0;
#endif

	/* Get code */
	while (isxdigit((int)(ch=buf[cpos++])) && 
	       isxdigit((int)(cl=buf[cpos++]))) {
	    byte_t byte = 0;
	    if (bytepos >= m->len) {
		if (report_error) {
		    fprintf(stderr,
			    "Error reading file. Invalid address. 0x%x\n",
			    bytepos);
		    fprintf(stderr, "Line %d:%s\n", lineno, buf);
		}
		return 0;
	    }
	    byte = hex2dig(ch)*16+hex2dig(cl);
	    m->contents[bytepos++] = byte;
	    byte_cnt++;
#ifdef HAS_GUI
	    empty_line = 0;
	    hexcode[index++] = ch;
	    hexcode[index++] = cl;
#endif
	}
#ifdef HAS_GUI
	/* Fill rest of hexcode with blanks */
	for (; index < 12; index++)
	    hexcode[index] = ' ';
	hexcode[index] = '\0';

	if (gui_mode) {
	    /* Now get the rest of the line */
	    while (isspace((int)buf[cpos]))
		cpos++;
	    cpos++; /* Skip over '|' */
	    
	    index = 0;
	    while ((c = buf[cpos++]) != '\0' && c != '\n') {
		line[index++] = c;
	    }
	    line[index] = '\0';
	    if (!empty_line)
		report_line(line_no++, addr, hexcode, line);
	}
#endif /* HAS_GUI */ 
    }
    return byte_cnt;
}

bool_t get_byte_val(mem_t m, word_t pos, byte_t *dest)
{
    if (pos < 0 || pos >= m->len)
	return FALSE;
    *dest = m->contents[pos];
    return TRUE;
}

bool_t get_word_val(mem_t m, word_t pos, word_t *dest)
{
    int i;
    word_t val;
    if (pos < 0 || pos + 4 > m->len)
	return FALSE;
    val = 0;
    for (i = 0; i < 4; i++)
	val = val | m->contents[pos+i]<<(8*i);
    *dest = val;
    return TRUE;
}

bool_t set_byte_val(mem_t m, word_t pos, byte_t val)
{
    if (pos < 0 || pos >= m->len)
	return FALSE;
    m->contents[pos] = val;
    return TRUE;
}

bool_t set_word_val(mem_t m, word_t pos, word_t val)
{
    int i;
    if (pos < 0 || pos + 4 > m->len)
	return FALSE;
    for (i = 0; i < 4; i++) {
	m->contents[pos+i] = val & 0xFF;
	val >>= 8;
    }
    return TRUE;
}

void dump_memory(FILE *outfile, mem_t m, word_t pos, int len)
{
    int i, j;
    while (pos % BPL) {
	pos --;
	len ++;
    }

    len = ((len+BPL-1)/BPL)*BPL;

    if (pos+len > m->len)
	len = m->len-pos;

    for (i = 0; i < len; i+=BPL) {
	word_t val = 0;
	fprintf(outfile, "0x%.4x:", pos+i);
	for (j = 0; j < BPL; j+= 4) {
	    get_word_val(m, pos+i+j, &val);
	    fprintf(outfile, " %.8x", val);
	}
    }
}

mem_t init_reg()
{
    return init_mem(32);
}

void free_reg(mem_t r)
{
    free_mem(r);
}

mem_t copy_reg(mem_t oldr)
{
    return copy_mem(oldr);
}

bool_t diff_reg(mem_t oldr, mem_t newr, FILE *outfile)
{
    word_t pos;
    int len = oldr->len;
    bool_t diff = FALSE;
    if (newr->len < len)
	len = newr->len;
    for (pos = 0; (!diff || outfile) && pos < len; pos += 4) {
	word_t ov, nv;
	get_word_val(oldr, pos, &ov);
	get_word_val(newr, pos, &nv);
	if (nv != ov) {
	    diff = TRUE;
	    if (outfile)
		fprintf(outfile, "%s:\t0x%.8x\t0x%.8x\n",
			reg_table[pos/4].name, ov, nv);
	}
    }
    return diff;
}

word_t get_reg_val(mem_t r, reg_id_t id)
{
    word_t val;
    if (id >= REG_NONE)
	return 0;
    get_word_val(r,id*4, &val);
    return val;
}

void set_reg_val(mem_t r, reg_id_t id, word_t val)
{
    if (id < REG_NONE) {
	set_word_val(r,id*4,val);
#ifdef HAS_GUI
	if (gui_mode) {
	    signal_register_update(id, val);
	}
#endif /* HAS_GUI */
    }
}
     
void dump_reg(FILE *outfile, mem_t r) {
    reg_id_t id;
    for (id = 0; id < 8; id++) {
	fprintf(outfile, "   %s  ", reg_table[id].name);
    }
    fprintf(outfile, "\n");
    for (id = 0; id < 8; id++) {
	word_t val;
	get_word_val(r, id*4, &val);
	fprintf(outfile, " %x", val);
    }
    fprintf(outfile, "\n");
}

struct {
    char symbol;
    int id;
} alu_table[A_NONE+1] = 
{
    {'+',   A_ADD},
    {'-',   A_SUB},
    {'^',   A_XOR},
    {'&',   A_AND},
    {'?',   A_NONE}
};

char op_name(alu_t op)
{
    if (op < A_NONE)
	return alu_table[op].symbol;
    else
	return alu_table[A_NONE].symbol;
}

word_t compute_alu(alu_t op, word_t argA, word_t argB)
{
    word_t val;
    switch(op) {
    case A_ADD:
	val = argA+argB;
	break;
    case A_SUB:
	val = argB-argA;
	break;
    case A_AND:
	val = argA&argB;
	break;
    case A_XOR:
	val = argA^argB;
	break;
	/* JB ajout SAL et SAR */
    case A_SAL:
	val = argB << argA;
	break;
    case A_SAR:
	val = argB >> argA;
	break;
    default:
	val = 0;
    }
    return val;
}

cc_t compute_cc(alu_t op, word_t argA, word_t argB)
{
    word_t val = compute_alu(op, argA, argB);
    bool_t zero = (val == 0);
    bool_t sign = ((int)val < 0);
    bool_t ovf;
    switch(op) {
    case A_ADD:
	ovf = (((int)argA < 0) == ((int)argB < 0)) &&
	    (((int)val < 0) != ((int)argA < 0));
	break;
    case A_SUB:
	ovf = (((int)argA > 0) == ((int)argB < 0)) &&
	    (((int)val < 0) != ((int)argB < 0));
	break;
	/* JB inutile
    case A_AND:
    case A_XOR:
	ovf = FALSE;
	break;
	*/
    default:
	ovf = FALSE;
    }
    return PACK_CC(zero,sign,ovf);
    
}

char *cc_names[8] = {
    "Z=0 S=0 O=0",
    "Z=0 S=0 O=1",
    "Z=0 S=1 O=0",
    "Z=0 S=1 O=1",
    "Z=1 S=0 O=0",
    "Z=1 S=0 O=1",
    "Z=1 S=1 O=0",
    "Z=1 S=1 O=1"};

char *cc_name(cc_t c)
{
    int ci = c;
    if (ci < 0 || ci > 7)
	return "???????????";
    else
	return cc_names[c];
}

/* Exception types */

char *exc_names[] = { "BUB", "AOK", "HLT", "ADR", "INS", "PIP" };

char *exc_name(exc_t e)
{
    if (e < 0 || e > EXC_PIPE)
	return "Invalid Exception";
    return exc_names[e];
}

/**************** Implementation of ISA model ************************/

state_ptr new_state(int memlen)
{
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = 0;
    result->r = init_reg();
    result->m = init_mem(memlen);
    result->cc = DEFAULT_CC;
    return result;
}

void free_state(state_ptr s)
{
    free_reg(s->r);
    free_mem(s->m);
    free((void *) s);
}

state_ptr copy_state(state_ptr s) {
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = s->pc;
    result->r = copy_reg(s->r);
    result->m = copy_mem(s->m);
    result->cc = s->cc;
    return result;
}

bool_t diff_state(state_ptr olds, state_ptr news, FILE *outfile) {
    bool_t diff = FALSE;

    if (olds->pc != news->pc) {
	diff = TRUE;
	if (outfile) {
	    fprintf(outfile, "pc:\t0x%.8x\t0x%.8x\n", olds->pc, news->pc);
	}
    }
    if (olds->cc != news->cc) {
	diff = TRUE;
	if (outfile) {
	    fprintf(outfile, "cc:\t%s\t%s\n", cc_name(olds->cc), cc_name(news->cc));
	}
    }
    if (diff_reg(olds->r, news->r, outfile))
	diff = TRUE;
    if (diff_mem(olds->m, news->m, outfile))
	diff = TRUE;
    return diff;
}


/* Branch logic */
bool_t take_branch(cc_t cc, jump_t bcond) {
    bool_t zf = GET_ZF(cc);
    bool_t sf = GET_SF(cc);
    bool_t of = GET_OF(cc);
    bool_t jump = FALSE;
    
    switch(bcond) {
    case J_YES:
	jump = TRUE;
	break;
    case J_LE:
	jump = (sf^of)|zf;
	break;
    case J_L:
	jump = sf^of;
	break;
    case J_E:
	jump = zf;
	break;
    case J_NE:
	jump = zf^1;
	break;
    case J_GE:
	jump = sf^of^1;
	break;
    case J_G:
	jump = (sf^of^1)&(zf^1);
	break;
    default:
	jump = FALSE;
	break;
    }
    return jump;
}


/* Execute single instruction.  Return exception condition. */
exc_t step_state(state_ptr s, FILE *error_file)
{
    word_t argA, argB;
    byte_t byte0 = 0;
    byte_t byte1 = 0;
    itype_t hi0;
    alu_t  lo0;
    reg_id_t hi1 = REG_NONE;
    reg_id_t lo1 = REG_NONE;
    bool_t ok1 = TRUE;
    word_t cval;
    word_t okc = TRUE;
    word_t val, dval;
    bool_t need_regids;
    bool_t need_imm;

    word_t ftpc = s->pc;

    if (!get_byte_val(s->m, ftpc, &byte0)) {
	if (error_file)
	    fprintf(error_file,
		    "PC = 0x%x, Invalid instruction address\n", s->pc);
	return EXC_ADDR;
    }
    ftpc++;

    hi0 = HI4(byte0);
    lo0 = LO4(byte0);

    need_regids =
	(hi0 == I_RRMOVL || hi0 == I_ALU || hi0 == I_PUSHL ||
	 hi0 == I_POPL || hi0 == I_IRMOVL || hi0 == I_RMMOVL ||
	 hi0 == I_MRMOVL || hi0 == I_ALUI);

    if (need_regids) {
	ok1 = get_byte_val(s->m, ftpc, &byte1);
	ftpc++;
	hi1 = HI4(byte1);
	lo1 = LO4(byte1);
    }

    need_imm =
	(hi0 == I_IRMOVL || hi0 == I_RMMOVL || hi0 == I_MRMOVL ||
	 hi0 == I_JXX || hi0 == I_CALL || hi0 == I_ALUI);

    if (need_imm) {
	okc = get_word_val(s->m, ftpc, &cval);
	ftpc += 4;
    }

    switch (hi0) {
    case I_NOP:
	s->pc = ftpc;
	break;
    case I_HALT:
	s->pc = ftpc;
	return EXC_HALT;
	break;
    case I_RRMOVL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (hi1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return EXC_INSTR;
	}
	if (lo1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, lo1);
	    return EXC_INSTR;
	}
	val = get_reg_val(s->r, hi1);
	set_reg_val(s->r, lo1, val);
	s->pc = ftpc;
	break;
    case I_IRMOVL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address",
			s->pc);
	    return EXC_INSTR;
	}
	if (lo1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, lo1);
	    return EXC_INSTR;
	}
	set_reg_val(s->r, lo1, cval);
	s->pc = ftpc;
	break;
    case I_RMMOVL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_INSTR;
	}
	if (hi1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return EXC_INSTR;
	}
	if (lo1 < 8) 
	    cval += get_reg_val(s->r, lo1);
	val = get_reg_val(s->r, hi1);
	if (!set_word_val(s->m, cval, val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid data address 0x%x\n",
			s->pc, cval);
	    return EXC_ADDR;
	}
	s->pc = ftpc;
	break;
    case I_MRMOVL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction addres\n", s->pc);
	    return EXC_INSTR;
	}
	if (hi1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return EXC_INSTR;
	}
	if (lo1 < 8) 
	    cval += get_reg_val(s->r, lo1);
	if (!get_word_val(s->m, cval, &val))
	    return EXC_ADDR;
	set_reg_val(s->r, hi1, val);
	s->pc = ftpc;
	break;
    case I_ALU:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	argA = get_reg_val(s->r, hi1);
	argB = get_reg_val(s->r, lo1);
	val = compute_alu(lo0, argA, argB);
	set_reg_val(s->r, lo1, val);
	s->cc = compute_cc(lo0, argA, argB);
	s->pc = ftpc;
	break;
    case I_JXX:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (take_branch(s->cc, lo0))
	    s->pc = cval;
	else
	    s->pc = ftpc;
	break;
    case I_CALL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	val = get_reg_val(s->r, REG_ESP) - 4;
	set_reg_val(s->r, REG_ESP, val);
	if (!set_word_val(s->m, val, ftpc)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid stack address 0x%x\n", s->pc, val);
	    return EXC_ADDR;
	}
	s->pc = cval;
	break;
    case I_RET:
	/* Return Instruction.  Pop address from stack */
	dval = get_reg_val(s->r, REG_ESP);
	if (!get_word_val(s->m, dval, &val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid stack address 0x%x\n",
			s->pc, dval);
	    return EXC_ADDR;
	}
	set_reg_val(s->r, REG_ESP, dval + 4);
	s->pc = val;
	break;
    case I_PUSHL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (hi1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n", s->pc, hi1);
	    return EXC_INSTR;
	}
	val = get_reg_val(s->r, hi1);
	dval = get_reg_val(s->r, REG_ESP) - 4;
	set_reg_val(s->r, REG_ESP, dval);
	if  (!set_word_val(s->m, dval, val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid stack address 0x%x\n", s->pc, dval);
	    return EXC_ADDR;
	}
	s->pc = ftpc;
	break;
    case I_POPL:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (hi1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n", s->pc, hi1);
	    return EXC_INSTR;
	}
	dval = get_reg_val(s->r, REG_ESP);
	set_reg_val(s->r, REG_ESP, dval+4);
	if (!get_word_val(s->m, dval, &val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid stack address 0x%x\n",
			s->pc, dval);
	    return EXC_ADDR;
	}
	set_reg_val(s->r, hi1, val);
	s->pc = ftpc;
	break;
    case I_LEAVE:
	dval = get_reg_val(s->r, REG_EBP);
	set_reg_val(s->r, REG_ESP, dval+4);
	if (!get_word_val(s->m, dval, &val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid stack address 0x%x\n",
			s->pc, dval);
	    return EXC_ADDR;
	}
	set_reg_val(s->r, REG_EBP, val);
	s->pc = ftpc;
	break;
    case I_ALUI:
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address\n", s->pc);
	    return EXC_ADDR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid instruction address",
			s->pc);
	    return EXC_INSTR;
	}
	if (lo1 >= 8) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%x, Invalid register ID 0x%.1x\n",
			s->pc, lo1);
	    return EXC_INSTR;
	}
	argB = get_reg_val(s->r, lo1);
	val = compute_alu(lo0, cval, argB);
	set_reg_val(s->r, lo1, val);
	s->cc = compute_cc(A_ADD, cval, argB);
	s->pc = ftpc;
	break;
    default:
	if (error_file)
	    fprintf(error_file,
		    "PC = 0x%x, Invalid instruction %.2x\n", s->pc, byte0);
	return EXC_INSTR;
    }
    return EXC_NONE;
}
