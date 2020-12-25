package keys;

import java.io.*;
import java.net.Socket;
import java.util.Scanner;

public class Client {
	
	public static void main(String[] args) throws IOException {
		System.out.print("Enter key:\n> ");
		String key = new Scanner(System.in).nextLine();
		
		Socket socket = new Socket("localhost", 8888);
		OutputStream out = socket.getOutputStream();
		PrintStream ps = new PrintStream(out, true);
		ps.println(key);
		
		InputStream in = socket.getInputStream();
		BufferedReader br = new BufferedReader(new InputStreamReader(in));
		String answer = br.readLine();
		
		if (!answer.equals("Allowed"))
			throw new SecurityException("Wrong key provided");
		
		System.out.println("Key is accepted!");
	}
}
