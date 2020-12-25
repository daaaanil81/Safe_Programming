package keys;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.util.Arrays;

public class ReadKey {
	
	public static void main(String[] args) throws Throwable {
		byte[] file = Files.readAllBytes(Paths.get("LKey.txt"));
		int newline = indexOf(file, (byte) 10);
		System.out.println("User: " + new String(Arrays.copyOfRange(file, 0, newline)));
		byte[] digitalSignature = Arrays.copyOfRange(file, newline + 1, file.length);
		
		Signature signature = Signature.getInstance("SHA256WithDSA");
		signature.initVerify(readKeys().getPublic());
		
		byte[] data = "wowsuchsignmuchsecurityverywow".getBytes(StandardCharsets.UTF_8);
		signature.update(data);
		
		System.out.println("Signature is " + (signature.verify(digitalSignature) ? "" : "in") + "valid.");
	}
	
	public static KeyPair readKeys() throws Throwable {
		ObjectInputStream ois = new ObjectInputStream(new FileInputStream("KeyPair.txt"));
		KeyPair keys = (KeyPair) ois.readObject();
		ois.close();
		return keys;
	}
	
	public static int indexOf(byte[] array, byte key) {
		int returnvalue = -1;
		for (int i = 0; i < array.length; ++i) {
			if (key == array[i]) {
				returnvalue = i;
				break;
			}
		}
		return returnvalue;
	}
}
