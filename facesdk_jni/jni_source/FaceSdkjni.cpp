#define LOG_TAG "FaceSdk"
#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include <math.h>
#include "cn_sensetime_felix_FaceSDK.h"
#include <assert.h>
#include "jni_common.h"

#include <mcv_facesdk.h>
#include <mcv_faceverify.h>

struct Handlers{
	mcv_handle_t detector;	//Detect handler
	mcv_handle_t vinst;		//extract handler
	mcv_handle_t tracker;   //tracker handler
};
/*
 * Class:     cn_sensetime_felix_FaceSDK
 * Method:    doInitResource
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_cn_sensetime_felix_FaceSDK_doInitResource
		(JNIEnv *env, jobject thiz, jstring prefix)
{
	Handlers* h = new Handlers;

	h->detector = mcv_facesdk_create_frontal_detector_instance_from_resource(1);
	const char *_prefix = env->GetStringUTFChars(prefix, 0);
	h->vinst = mcv_create_verify_instance_with_prefix(_prefix);
	h->tracker = mcv_facesdk_create_multi_face_tracker(1);
	if(!h->detector || !h->vinst || !h->tracker) {
		if (h->detector) mcv_facesdk_destroy_frontal_instance(h->detector);
		if (h->vinst) mcv_verify_release_instance(h->vinst);
		if (h->tracker) mcv_facesdk_destroy_multi_face_tracker(h->tracker);
		return JNI_FALSE;
	}
	setHandle(env, thiz, h);
	return JNI_TRUE;
}

/*
 * Class:     cn_sensetime_felix_FaceSDK
 * Method:    doDetect
 * Signature: ([BII)[I
 */
JNIEXPORT jintArray JNICALL Java_cn_sensetime_felix_FaceSDK_doDetect
  (JNIEnv *env, jobject thiz, jbyteArray data, jint width, jint height)
{
	mcv_handle_t handle = getHandle<Handlers>(env, thiz)->detector;

	//raw array
	unsigned char *pixels = NULL;
	pixels = (unsigned char *)env->GetByteArrayElements(data, 0);

	PMCV_FACERECT rects;
	unsigned int rectsCount;

	mcv_result_t res = mcv_facesdk_frontal_detector(handle, (unsigned char*)pixels,
			width, height, width,
			&rects, &rectsCount);
	env->ReleaseByteArrayElements(data, (jbyte *)pixels, 0);

	size_t size = rectsCount * 6;

	jintArray sp=(jintArray)env->NewIntArray(size);
	jint*f = (jint*)malloc(size * sizeof(jint));
	if(!f) {
		mcv_facesdk_release_frontal_result(rects, rectsCount);
		return NULL;
	}

	for (size_t i = 0; i < rectsCount; i++) {
		f[6*i] = (rects[i].Rect.left);
		f[6*i+1] = (rects[i].Rect.top);
		f[6*i+2] = (rects[i].Rect.right);
		f[6*i+3] = (rects[i].Rect.bottom);
		f[6*i+4] = (rects[i].Pose);
		f[6*i+5] = (int)(rects[i].Confidence);
	}

	mcv_facesdk_release_frontal_result(rects, rectsCount);
	

	env->SetIntArrayRegion(sp, 0, size, f);
	free(f);
	return sp;
}

/*
* Class:     cn_sensetime_felix_FaceSDK
* Method:    doMultiTracker
* Signature: ([BII)[I
*/
JNIEXPORT jintArray JNICALL Java_cn_sensetime_felix_FaceSDK_doMultiTracker
(JNIEnv *env, jobject thiz, jbyteArray data, jint orientation, jint width, jint height)
{
	mcv_handle_t handle = getHandle<Handlers>(env, thiz)->tracker;

	//raw array
	unsigned char *pixels = NULL;
	pixels = (unsigned char *)env->GetByteArrayElements(data, 0);

	PMCV_FACERECTID rectids;
	unsigned int rectsCount;

	mcv_result_t res = mcv_facesdk_multi_face_tracker_feed_gray_frame(handle, (unsigned char*)pixels,
		width, height, (MCV_FaceOrientation)orientation,
		&rectids, &rectsCount);
	if (res != MCV_OK){
		return NULL;
	}
	env->ReleaseByteArrayElements(data, (jbyte *)pixels, 0);

	size_t size = rectsCount * 5;

	jintArray sp = (jintArray)env->NewIntArray(size);
	jint*f = (jint*)malloc(size * sizeof(jint));
	if (!f) {
		mcv_facesdk_release_multitracker_result(rectids, rectsCount);
		return NULL;
	}

	for (size_t i = 0; i < rectsCount; i++) {
		f[5 * i] = (rectids[i].Rect.left);
		f[5 * i + 1] = (rectids[i].Rect.top);
		f[5 * i + 2] = (rectids[i].Rect.right);
		f[5 * i + 3] = (rectids[i].Rect.bottom);
		f[5 * i + 4] = (rectids[i].ID);
	}


	mcv_facesdk_release_multitracker_result(rectids, rectsCount);


	env->SetIntArrayRegion(sp, 0, size, f);
	free(f);
	return sp;
}


/*
* Class:     cn_sensetime_felix_FaceSDK
* Method:    doMultiTrackerAlgin
* Signature: ([BII)[I
*/
JNIEXPORT jfloatArray JNICALL Java_cn_sensetime_felix_FaceSDK_doTrackerAlgin
(JNIEnv *env, jobject thiz, jintArray faceRectid)
{
	mcv_handle_t handle = getHandle<Handlers>(env, thiz)->tracker;
	//face rect
	MCV_FACERECTID rectids = { 0 };
	jint *b = env->GetIntArrayElements(faceRectid, 0);
	mcv_pointf_t pts[MCV_MUTLITRACKER_ALIGN_POINTS];

	rectids.Rect.left = b[0];
	rectids.Rect.top = b[1];
	rectids.Rect.right = b[2];
	rectids.Rect.bottom = b[3];
	rectids.ID = b[4];

	mcv_result_t res = mcv_facesdk_multi_face_tracker_align(handle, &rectids, pts);
	if (res != MCV_OK){
		env->ReleaseIntArrayElements(faceRectid, b, 0);
		return NULL;
	}

	const size_t size = MCV_MUTLITRACKER_ALIGN_POINTS * 2;
	jfloatArray sp = (jfloatArray)env->NewFloatArray(size);
	//jfloat *f = (jfloat*)malloc(size * sizeof(jfloat));
	float f[size];

	for (size_t i = 0; i<MCV_MUTLITRACKER_ALIGN_POINTS; i++){
		f[2 * i + 0] = pts[i].x;
		f[2 * i + 1] = pts[i].y;
	}
	env->SetFloatArrayRegion(sp, 0, size, (jfloat*)f);
	env->ReleaseIntArrayElements(faceRectid, b, 0);
	return sp;
}



/*
* Class:     cn_sensetime_felix_FaceSDK
* Method:    doGetPose
* Signature: ([BII)[I
*/
JNIEXPORT jfloatArray JNICALL Java_cn_sensetime_felix_FaceSDK_doGetPose
(JNIEnv *env, jobject thiz, jfloatArray facial_points_array)
{
	jfloat *f = env->GetFloatArrayElements(facial_points_array, 0);
	mcv_pointf_t pts[MCV_MUTLITRACKER_ALIGN_POINTS];
	for (size_t i = 0; i < MCV_MUTLITRACKER_ALIGN_POINTS; i++){
		pts[i].x = f[2 * i + 0];
		pts[i].y = f[2 * i + 1];
	}
	float yaw, pitch, roll, eyedist;
	mcv_result_t res = mcv_facesdk_get_pose(pts, 21, &yaw, &pitch, &roll, &eyedist);
	if (res != MCV_OK){
		env->ReleaseFloatArrayElements(facial_points_array, f, 0);
		return NULL;
	}

	const size_t size = 4;
	jfloatArray sp = (jfloatArray)env->NewFloatArray(size);
	float f1[size];
	f1[0] = yaw;
	f1[1] = pitch;
	f1[2] = roll;
	f1[3] = eyedist;

	env->SetFloatArrayRegion(sp, 0, size, (jfloat*)f1);
	env->ReleaseFloatArrayElements(facial_points_array, f, 0);
	return sp;
}

/*
 * Class:     cn_sensetime_felix_FaceSDK
 * Method:    doExtract
 * Signature: ([BII[I)[F
 */
JNIEXPORT jfloatArray JNICALL Java_cn_sensetime_felix_FaceSDK_doExtract
  (JNIEnv *env, jobject thiz, jbyteArray data, jint width, jint height, jintArray face_rect)
{
		mcv_handle_t handle = getHandle<Handlers>(env, thiz)->vinst;
		if(!handle)
			return 0;
		unsigned char *srcp = NULL;
		srcp = (unsigned char *)env->GetByteArrayElements(data, 0);

		jint* rect = env->GetIntArrayElements(face_rect, 0);
		int face_rect_len = env->GetArrayLength(face_rect);
		if (face_rect_len != 4) {
			fprintf(stderr,"length is too short!\n");
		}

		mcv_rect_t r;
		r.left = rect[0];
		r.top = rect[1];
		r.right = rect[2];
		r.bottom = rect[3];
		float *feat = 0;
		int len = mcv_extract_pca_feature(handle, srcp, width, height, r, &feat);

		jfloatArray retv = env->NewFloatArray(len);
		env->SetFloatArrayRegion(retv, 0, len, (jfloat*)feat);

		mcv_verify_release_feature(feat);
		return retv;
}

/*
 * Class:     cn_sensetime_felix_FaceSdkjni
 * Method:    doVerify
 * Signature: ([F[F)F
 */
JNIEXPORT jfloat JNICALL Java_cn_sensetime_felix_FaceSDK_doVerify
  (JNIEnv *env, jobject thiz, jfloatArray face_feat1, jfloatArray face_feat2)
{
	mcv_handle_t handle = getHandle<Handlers>(env, thiz)->vinst;
	if(!handle)
		return 0;
	jsize len1 = env->GetArrayLength(face_feat1);
	jsize len2 = env->GetArrayLength(face_feat2);
	if(len1 != len2)
		return 0;
	jfloat *f1 = env->GetFloatArrayElements(face_feat1, 0);
	jfloat *f2 = env->GetFloatArrayElements(face_feat2, 0);
	float score = -2000;
	mcv_verify_pca_feature(handle, f1, len1, f2, len2, &score);
	env->ReleaseFloatArrayElements(face_feat1, f1, 0);
	env->ReleaseFloatArrayElements(face_feat2, f2, 0);

	return score;
}

/*
 * Class:     cn_sensetime_felix_FaceSdkjni
 * Method:    doDestroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_cn_sensetime_felix_FaceSDK_doDestroy
  (JNIEnv *env, jobject thiz)
{
	Handlers* h = getHandle<Handlers>(env, thiz);
	if(h) {
		if (h->detector) mcv_facesdk_destroy_frontal_instance(h->detector);
		if (h->vinst) mcv_verify_release_instance(h->vinst);
		if (h->tracker) mcv_facesdk_destroy_multi_face_tracker(h->tracker);
		h = NULL;
		setHandle(env, thiz, h);
	}
}



