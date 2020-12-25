package keys;

import java.io.*;
import java.security.*;

public class GenKey {
	
	public static void main(String[] args) throws Throwable {
		KeyPairGenerator gen = KeyPairGenerator.getInstance("DSA");
		gen.initialize(2048, new SecureRandom());
		KeyPair keys = gen.generateKeyPair();
		
		ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream("KeyPair.txt"));
		oos.writeObject(keys);
		oos.flush();
		oos.close();
	}
}