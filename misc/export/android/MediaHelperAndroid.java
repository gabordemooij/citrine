
/* Copy this to <AndroidProject>/app/src/main/java/org/libsdl/app/MediaHelperAndroid.java */

package org.libsdl.app;

import android.content.Context;

import java.lang.Class;
import java.lang.reflect.Method;

import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;

public class MediaHelperAndroid {

	public static String httpRequest(String urlstr, String data) throws Exception {
		String content = "", line;
		try {
			URL url = new URL(urlstr);
			HttpURLConnection connection = (HttpURLConnection) url.openConnection();
			connection.setDoOutput(true);
			if (data == null) {
				connection.setRequestMethod("GET");
			} else {
				connection.setRequestMethod("POST");
				BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(connection.getOutputStream()));
				writer.write(data);
				writer.flush();
				writer.close();
			}
			connection.setConnectTimeout(5000);
			connection.setReadTimeout(5000);
			connection.connect();
			BufferedReader rd = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			while ((line = rd.readLine()) != null) {
				content += line + "\n";
			}
		} catch (Exception ex) {
				return ex.getMessage();
		}
		return content;
	}
}