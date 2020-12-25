package keys;

import java.io.*;
import java.security.*;
import java.util.Scanner;

public class MakeKey {
	
	public static void main(String[] args) throws Throwable {
		Signature signature = Signature.getInstance("SHA256WithDSA");
		signature.initSign(readKeys().getPrivate(), new SecureRandom());
		
		byte[] data = "wowsuchsignmuchsecurityverywow".getBytes();
		signature.update(data);
		
		System.out.print("Enter username:\n> ");
		String user = new Scanner(System.in).nextLine();
		
		FileOutputStream fos = new FileOutputStream("LKey.txt");
		fos.write(user.getBytes());
		fos.write(10); // new line
		fos.write(signature.sign());
		
		fos.flush();
		fos.close();
		
		System.out.println("Licence key is saved in LKey.txt");
	}
	
	public static KeyPair readKeys() throws Throwable {
		ObjectInputStream ois = new ObjectInputStream(new FileInputStream("KeyPair.txt"));
		KeyPair keys = (KeyPair) ois.readObject();
		ois.close();
		return keys;
	}
}
