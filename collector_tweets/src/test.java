import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;


public class test {

	public static void main(String[] args) {
		System.out.println("\n".length());
		// TODO Auto-generated method stub
//		System.out.println(String.format("%" + 5 + "st", "asdfgfdg"));
//		String s = "!@#$%^&*()";
//		s = s.replaceAll("[^A-Za-z0-9 !@#$%^&*()]", "");
//		System.out.println(s);
		RandomAccessFile outputFile;
		try {
			outputFile = new RandomAccessFile(new File("file.txt"), "rwd");
			//outputFile.readLong();//writeLong(123);
//			outputFile.writeLong(456);
//			outputFile.writeBytes("test");
			//outputFile.seek(0);
			byte[] l = new byte [5];
			outputFile.read(l);
			String file_string = "";

		    for(int i = 0; i < l.length; i++)
		    {
		        file_string += (char)l[i];
		    }
		    long lqwe = Long.parseLong(new String(l, "UTF-8").trim());
			System.out.println(lqwe);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
