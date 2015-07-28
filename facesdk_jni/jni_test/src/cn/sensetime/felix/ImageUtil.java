package cn.sensetime.felix;

//import java.awt.Image;
import java.lang.String;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

public class ImageUtil {
	/**
	 * 
	 * 加载图片
	 * 
	 * @param imgpath
	 * 图片路径（相对路径与绝对路径均可）
	 * 
	 * @return
	 * 获取到的图片实例
	 * 
	 */
	public static BufferedImage getimage(String imgpath) throws IOException
	{
		BufferedImage img = null;	
		img = ImageIO.read(new File(imgpath));
		
		return img;
	}
	
	/**
	 * 
	 * 获取BGRA像素的字节数组
	 * 
	 * @param img
	 * 图片实例
	 * 
	 * @return
	 * 返回字节数组
	 * 
	 */
	public static byte[] GetBGRAPixels(BufferedImage img)
	{
		int w = img.getWidth();
		int h = img.getHeight();
		byte[] pixels = new byte[w*h*4];
		
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
            	int color =  img.getRGB(i, j);
            	pixels[(j*w+i)*4+0] = (byte)((color & 0x000000FF));
            	pixels[(j*w+i)*4+1] = (byte)((color & 0x0000FF00) >> 8);
            	pixels[(j*w+i)*4+2] = (byte)((color & 0x00FF0000) >> 16);
            	pixels[(j*w+i)*4+3] = (byte)((color & 0xFF000000) >> 24);
            }
        }
        return pixels;
	}
	
	/**
	 * 
	 * 获取Gray灰度图像素的字节数组
	 * 
	 * @param img
	 * 图片实例
	 * 
	 * @return
	 * 返回字节数组
	 * 
	 */
	public static byte[] GetGrayPixels(BufferedImage img)
	{
		int w = img.getWidth();
		int h = img.getHeight();
		byte[] pixels = new byte[w*h];
		for(int i = 0; i < h; i++)
		{
			for(int j = 0; j < w; j++)
			{
				//获取像素颜色
				int color = img.getRGB(j, i);
				int alpha = (color & 0xFF000000) >> 24;
				int red = (int)(((color & 0x00FF0000) >> 16) * 0.3);
				int green = (int)(((color & 0x0000FF00) >> 8) * 0.59);
				int blue = (int)((color & 0x000000FF) * 0.11);
				//灰度值
				color = red + green + blue;
				pixels[w * i + j] = (byte)color;
			}
		}
        return pixels;
	}
	
}