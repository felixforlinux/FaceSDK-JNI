/*
 * =====================================================================================
 *
 *       Filename:  jni_common.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/12/2013 05:31:16 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Chen Yuheng (Chen Yuheng), chyh1990@163.com
 *   Organization:  Tsinghua Unv.
 *
 * =====================================================================================
 */
#ifndef _HANDLE_H_INCLUDED_
#define _HANDLE_H_INCLUDED_
#include <jni.h>



#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

static inline jfieldID getHandleField(JNIEnv *env, jobject obj)
{
    jclass c = env->GetObjectClass(obj);
    // J is the type signature for long:
    return env->GetFieldID(c, "nativeHandle", "J");
}

template <typename T>
T *getHandle(JNIEnv *env, jobject obj)
{
    jlong handle = env->GetLongField(obj, getHandleField(env, obj));
    return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t)
{
    jlong handle = reinterpret_cast<jlong>(t);
    env->SetLongField(obj, getHandleField(env, obj), handle);
}

#endif
