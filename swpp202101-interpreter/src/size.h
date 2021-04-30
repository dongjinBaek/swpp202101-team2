#ifndef SWPP_ASM_INTERPRETER_SIZE_H
#define SWPP_ASM_INTERPRETER_SIZE_H


enum MSize {
  MSize1 = 0,
  MSize2,
  MSize4,
  MSize8
};

enum Size{
  Size1 = 0,
  Size8,
  Size16,
  Size32,
  Size64
};

enum VSize {
  VSize2 = 0,
  VSize4,
  VSize8
};

int msize_of(MSize msize);

int bw_of(Size size);

int vsize_of(VSize vsize);

extern double _ret, _bruncond, _brcond_true, _brcond_false, _switch, _call;
extern double _call_arg, _malloc, _free, _load_stack, _load_heap, _store_stack;
extern double _store_heap, _temp, _vload_stack, _vload_heap, _vstore_stack;
extern double _vstore_heap, _vtemp, _cool, _muldiv, _logical, _addsub, _comp;
extern double _ternary, _read, _write;

#endif //SWPP_ASM_INTERPRETER_SIZE_H
