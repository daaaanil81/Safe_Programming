package keys;

import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.util.List;

public class Server {
	
	public static List<String> keys;
	
	public static void main(String[] args) throws IOException {
		keys = Files.readAllLines(Paths.get("Keys.txt"));
		
		ServerSocket server = new ServerSocket(8888);
		
		while (true) {
			Socket socket = server.accept();
			
			BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			String key = br.readLine();
			System.out.println("Received key: " + key);
			
			PrintStream ps = new PrintStream(socket.getOutputStream(), true);
			ps.println(keys.contains(key) ? "Allowed" : "Forbidden");
		}
	}
}
