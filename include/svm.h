#ifndef __SVM_H__
#define __SVM_H__


#include "types.h"
#include "cpu.h"
#include "vmcb.h"


extern void __init enable_svm ( struct cpuinfo_x86 *c );
extern void svm_launch ( u64 vmcb );


#endif /* __SVM_H__ */
