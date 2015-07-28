package cn.sensetime.felix;

import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorConvertOp;
import java.io.*;
import java.lang.String;
import java.util.*;


public class FaceSDK {
	private long nativeHandle;
	static{
        System.loadLibrary("x64/facesdk");
        System.loadLibrary("x64/faceverify");
		System.loadLibrary("x64/facesdk-jni");
	}
	
// for 32bit support
//	static{
//        System.loadLibrary("x86/facesdk");
//        System.loadLibrary("x86/faceverify");
//		System.loadLibrary("x86/facesdk-jni");
//	}

	public FaceSDK()
	{
		doInitResource("data/");
	}
	public FaceSDK(String prefix)
	{
		doInitResource(prefix);
	}
	/**
	 * 
	 * 对图片进行人脸检测，并返回人脸框信息
	 * 
	 * @param img
	 * 图片实例
	 * 
	 * @return
	 * 人脸框信息数组
	 * 
	 */
	public  FaceRect[] Detect(BufferedImage img) throws IOException
	{	
		int width = img.getWidth();
		int height = img.getHeight();
		BufferedImage grayImage = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_GRAY);//转为灰度图
		ColorConvertOp cco = new ColorConvertOp(ColorSpace.getInstance(ColorSpace.CS_GRAY), null);  
        cco.filter(img, grayImage); 
        
		byte[] imgpixels = ImageUtil.GetGrayPixels(grayImage);	
		
		int[] detinfo = doDetect(imgpixels, width, height);
		if(detinfo == null)
			return null;
		FaceRect[] ret = new FaceRect[detinfo.length / 6];
		for (int i = 0; i < detinfo.length / 6; i++) {
			ret[i] = new FaceRect();
			ret[i].left	= detinfo[i*6];
			ret[i].top	= detinfo[i*6+1];
			ret[i].right = detinfo[i*6+2];
			ret[i].bottom = detinfo[i*6+3];
			ret[i].score = detinfo[6*i+4];
		}
		
		return ret;		
	}
	
	/**
	 * 
	 * 人脸跟踪，返回人脸框信息
	 * 
	 * @param img
	 * 图片实例
	 *   
	 * @return
	 * 返回人脸框FaceRectID
	 * 
	 */	
	public  FaceRectId[] Tracker(BufferedImage img) throws IOException
	{
		int width = img.getWidth();
		int height = img.getHeight();
		BufferedImage grayImage = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_GRAY);//转为灰度图
		ColorConvertOp cco = new ColorConvertOp(ColorSpace.getInstance(ColorSpace.CS_GRAY), null);  
        cco.filter(img, grayImage); 
        
		byte[] imgpixels = ImageUtil.GetGrayPixels(grayImage);	
		
		int[] detinfo = doMultiTracker(imgpixels, 0, width, height);
		if(detinfo == null)
			return null;
		FaceRectId[] ret = new FaceRectId[detinfo.length / 5];
		for (int i = 0; i < detinfo.length / 5; i++) {
			ret[i] = new FaceRectId();
			ret[i].left	= detinfo[i*6];
			ret[i].top	= detinfo[i*6+1];
			ret[i].right = detinfo[i*6+2];
			ret[i].bottom = detinfo[i*6+3];
			ret[i].id = detinfo[6*i+4];
		}
		
		return ret;		
	}
	
	/**
	 * 
	 * 对tracker返回值做关键点检测
	 * 
	 * @param facerectid
	 * tracker返回的人脸框信息
	 *   
	 * @return
	 * 返回关键点坐标数组
	 * 
	 */	
	public  Point[] TrackerAlgin(FaceRectId facerectid) throws IOException
	{
		Point[] pt = new Point[21];
		float[] pts = new float[2*21];
		int [] faceid = new int[5];
		faceid[0] = facerectid.left;
		faceid[1] = facerectid.top;
		faceid[2] = facerectid.right;
		faceid[3] = facerectid.bottom;
		faceid[4] = facerectid.id;
		pts = doTrackerAlgin(faceid);
		for(int i = 0; i<21; i++)
		{
			pt[i] = new Point();
			pt[i].x = pts[2 * i + 0];
			pt[i].y = pts[2 * i + 1];
		}
		return pt;
	}
	
	/**
	 * 
	 * 根据21关键点获取pose信息
	 * 
	 * @param pt
	 * 关键点坐标
	 *   
	 * @return
	 * 返回pose信息
	 * pose[0]:用于保存水平转角
	 * pose[1]:用于保存俯仰角
	 * pose[2]:用于保存旋转角
	 * pose[3]:用于保存两眼距
	 * 
	 */	
	public  float[] GetPose(Point[] pt) throws IOException
	{
		float[] pose = new float[4];
		float[] pts = new float[42];
		for(int i = 0; i<21; i++)
		{
			pts[2 * i + 0] = pt[i].x;
			pts[2 * i + 1] =pt[i].y;
		}
		
		pose= doGetPose(pts);
		return pose;
	}
	
	
	/**
	 * 
	 * 提取face feature
	 * 
	 * @param img
	 * 图片实例
	 * 
	 * @param rect
	 * 人脸框信息数组
	 * 
	 * @return
	 * 返回feature数组
	 * 
	 */
	public float[] Extract(BufferedImage img, FaceRect rect) throws IOException
	{
		int width = img.getWidth();
		int height = img.getHeight();
		byte[] imgbyte = ImageUtil.GetBGRAPixels(img);
		 
		int[] facerect = new int[4];
		facerect[0] = rect.left;
		facerect[1] = rect.top;
		facerect[2] = rect.right;
		facerect[3] = rect.bottom;
		
		float[] ret = doExtract(imgbyte, width, height, facerect);
	
		return ret;	
	}
	
	/**
	 * 
	 * 人脸比对，返回比对得分
	 * 
	 * @param face_feat1
	 * 人脸feature
	 * @param face_feat2
	 * 人脸feature
	 *  
	 * @return
	 * 返回得分，得分范围是0～1，参考阈值可以设置在0.5~0.9视具体情况而定，建议可以使用0.5的阈值
	 * 
	 */
	public float Verify(float[] face_feat1, float[] face_feat2)
	{	
		return doVerify(face_feat1, face_feat2);
	}
	
	

	/**
	 * 
	 * 释放资源
	 * 
	 */
	protected void finalize()
	{
		doDestroy();
	}
	
	/**
	 * 
	 * 用于JNI封装
	 * 
	 */
	private native boolean doInitResource(String prefix);
	private native int[] doDetect(byte[] data, int width, int height);
	private native float[] doExtract(byte[] data, int width, int height, int[] face_rect);
	private native float doVerify(float[] face_feat1, float[] face_feat2);
	private native int[] doMultiTracker(byte[] data, int orientation, int width, int height);
	private native float[] doTrackerAlgin(int[] faceRectid);
	private native float[] doGetPose(float[] facial_points_array);
	private native void doDestroy();

	
	/**
	 * 
	 * Sample Code 请在使用前删除 TODO
	 * 
	 */
	public static void main(String[] args) throws IOException
	{
		/**
		 * 获取图片实例
		 * */
		BufferedImage img1 = ImageUtil.getimage("images/1.jpg");
	//	BufferedImage img2 = ImageUtil.getimage("images/2.jpg");
		
		FaceSDK sdk = new FaceSDK();
		/**
		 * 对连续帧对人脸洁厕（本例用图片代替）
		 * */
		for(int i = 0; i<5; i++)
		{
			//获取人脸信息
			FaceRectId[] face1 = sdk.Tracker(img1);
			for(int j = 0; j<face1.length; j++)
			{
				Point[] pt = new Point[21];
				System.out.println("left:" + face1[j].left + " " + "top:" + face1[j].top + " " + "right:" + face1[j].right + " " + "bottom:"+ face1[j].bottom);
				//获取关键点
				pt = sdk.TrackerAlgin(face1[j]);
				for(int k = 0; k <5; k++)
				{
					System.out.println("x:"+pt[k].x +" "+"y:"+pt[k].y);
				}
				float [] pose = new float[4];
				//获取朝向
				pose = sdk.GetPose(pt);
				System.out.println("poseinfo->" + "yaw : "+ pose[0] + " " + "pitch:" +pose[1] + " " + "roll:" +pose[2] + " " +"eyedist:" + pose[3]);
			}
			

		}
		
	
		//FaceRect[] face2 = sdk.Detect(img2);
		/**
		 * 图片img中若检测到人脸，则提取人脸feature，并返回feature；若未检测到人脸，则输出"Face not found！"
		 * */
		//if (face1 != null && face2 != null) {
		//	float[] f1 = sdk.Extract(img1, face1[0]);
		//	float[] f2 = sdk.Extract(img2, face2[0]);
			/**
			 * 进行人脸比对，获取人脸比对的结果得分
			 * */
		//	float score = sdk.Verify(f1, f2);	
		//	System.out.println("结果得分为： " + score);
		//} else {
		//	System.err.println("Face not found！");
		//}
	}
	
}
