
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

import android.util.Log;


/* vault */
import android.content.SharedPreferences;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.util.Base64;

import java.nio.charset.StandardCharsets;
import java.security.KeyStore;
import java.security.SecureRandom;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;


/* /vault */

public class MediaHelperAndroid {
	
	private static final String KEY_ALIAS = "XoScriptSecureKey";
    private static final String ANDROID_KEYSTORE = "AndroidKeyStore";
    private static final String PREF_NAME = "xo_secure_prefs";


    private static final int IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;
    
    
    private static SecretKey getOrCreateSecretKey() throws Exception {
        KeyStore keyStore = KeyStore.getInstance(ANDROID_KEYSTORE);
        keyStore.load(null);
        if (!keyStore.containsAlias(KEY_ALIAS)) {
            KeyGenerator keyGenerator = KeyGenerator.getInstance(
                    KeyProperties.KEY_ALGORITHM_AES, ANDROID_KEYSTORE);
            keyGenerator.init(
                    new KeyGenParameterSpec.Builder(KEY_ALIAS,
                            KeyProperties.PURPOSE_ENCRYPT | KeyProperties.PURPOSE_DECRYPT)
                            .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                            .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                            .setKeySize(256)
                            .build()
            );
            keyGenerator.generateKey();
        }
        return ((SecretKey) keyStore.getKey(KEY_ALIAS, null));
    }


	public static void storeToken(Context context, String name, String token) {
    try {
        SecretKey secretKey = getOrCreateSecretKey();
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, secretKey);
        byte[] iv = cipher.getIV();
        byte[] encryptedBytes = cipher.doFinal(token.getBytes(StandardCharsets.UTF_8));
        String encryptedBase64 = Base64.encodeToString(encryptedBytes, Base64.DEFAULT);
        String ivBase64 = Base64.encodeToString(iv, Base64.DEFAULT);
        SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
        prefs.edit()
                .putString(name + "_data", encryptedBase64)
                .putString(name + "_iv", ivBase64)
                .apply();
    } catch (Exception e) {
        Log.e("CITRINE", "Failed to encrypt: " + e.getMessage(), e);
    }
}
    
    public static String getToken(Context context, String name) {
        try {
            SharedPreferences prefs = context.getSharedPreferences(PREF_NAME, Context.MODE_PRIVATE);
            String encryptedBase64 = prefs.getString(name + "_data", null);
            String ivBase64 = prefs.getString(name + "_iv", null);
            if (encryptedBase64 == null || ivBase64 == null) return null;
			byte[] encryptedBytes = Base64.decode(encryptedBase64, Base64.DEFAULT);
            byte[] iv = Base64.decode(ivBase64, Base64.DEFAULT);
			SecretKey secretKey = getOrCreateSecretKey();
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
            cipher.init(Cipher.DECRYPT_MODE, secretKey, spec);
            byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
            return new String(decryptedBytes, StandardCharsets.UTF_8);

        } catch (Exception e) {
            Log.e("CITRINE", "Failed to decrypt: " + e.getMessage(), e);
            return null;
        }
    }
    
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