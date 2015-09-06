#ifndef __SIFUNC_H__
#define __SIFUNC_H__

#define VA_NUM_ARGS2(...) VA_NUM_ARGS2_IMPL(0, ## __VA_ARGS__, 8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0)
#define VA_NUM_ARGS2_IMPL(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define RECURSIVE2(b,r,...) RECURSIVE2_N(b,r,VA_NUM_ARGS2(__VA_ARGS__),__VA_ARGS__)
#define RECURSIVE2_N(b,r,n,...) CAT(RECURSIVE2_,n)(b,r,__VA_ARGS__)
#define RECURSIVE2_8(b,r,t,n,...) r(t,n) RECURSIVE2_7(b,r,__VA_ARGS__)
#define RECURSIVE2_7(b,r,t,n,...) r(t,n) RECURSIVE2_6(b,r,__VA_ARGS__)
#define RECURSIVE2_6(b,r,t,n,...) r(t,n) RECURSIVE2_5(b,r,__VA_ARGS__)
#define RECURSIVE2_5(b,r,t,n,...) r(t,n) RECURSIVE2_4(b,r,__VA_ARGS__)
#define RECURSIVE2_4(b,r,t,n,...) r(t,n) RECURSIVE2_3(b,r,__VA_ARGS__)
#define RECURSIVE2_3(b,r,t,n,...) r(t,n) RECURSIVE2_2(b,r,__VA_ARGS__)
#define RECURSIVE2_2(b,r,t,n,...) r(t,n) RECURSIVE2_1(b,r,__VA_ARGS__)
#define RECURSIVE2_1(b,r,t,n,...) b(t,n)
#define RECURSIVE2_0(b,r,...)

#define PARAMS_BASE(t,n) t n
#define PARAMS_REC(t,n) PARAMS_BASE(t,n) ,
#define PARAMS(...) RECURSIVE2(PARAMS_BASE,PARAMS_REC,__VA_ARGS__)

#define PARAM_TYPES_BASE(t,n) t
#define PARAM_TYPES_REC(t,n) t ,
#define PARAM_TYPES(...) RECURSIVE2(PARAM_TYPES_BASE,PARAM_TYPES_REC,__VA_ARGS__)

#define PARAM_NAMES_BASE(t,n) n
#define PARAM_NAMES_REC(t,n) PARAM_NAMES_BASE(t,n) ,
#define PARAM_NAMES(...) RECURSIVE2(PARAM_NAMES_BASE,PARAM_NAMES_REC,__VA_ARGS__)

#define PARAM_CONST_BASE(t,n) __builtin_constant_p(n) &&
#define PARAM_CONST_REC(t,n) PARAM_CONST_BASE(t,n)
#define PARAM_CONST(N,...) RECURSIVE2_N(PARAM_CONST_BASE,PARAM_CONST_REC,N,__VA_ARGS__) 1

#define INLINE __attribute__((always_inline)) inline

/*
IIFUNC(ret_type,name,nconst,type1,arg1,type2,arg2,...)
  - ret_type: function return type
  - name:     function name
  - nconst:   number og arguments to include into the const check
  - type<x>:  argument type
  - arg<x>:   argument name
*/
#define SIFUNC_H(ret_type,name,nconst,...) \
INLINE ret_type name(PARAMS(__VA_ARGS__));        \
INLINE ret_type _##name##_c(PARAMS(__VA_ARGS__)); \
ret_type _##name##_v(PARAMS(__VA_ARGS__)); \
ret_type name(PARAMS(__VA_ARGS__)) { \
  if(PARAM_CONST(nconst,__VA_ARGS__)) { \
    return _##name##_c(PARAM_NAMES(__VA_ARGS__)); \
  } else { \
    return _##name##_v(PARAM_NAMES(__VA_ARGS__)); \
  } \
} \
ret_type _##name##_c(PARAMS(__VA_ARGS__))

#define SIFUNC_CPP(ret_type,name,nconst,...) \
ret_type _##name##_v(PARAMS(__VA_ARGS__)) { \
  return _##name##_c(PARAM_NAMES(__VA_ARGS__)); \
}

#define SIFUNC(ret_type,name,nconst,...) \
SIFUNC_H(ret_type,name,nconst,__VA_ARGS__) \
SIFUNC_CPP(ret_type,name,nconst,__VA_ARGS__)

#endif
