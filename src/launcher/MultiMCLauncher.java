import java.applet.Applet;
import java.awt.Dimension;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Modifier;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

public class MultiMCLauncher
{
	/**
	 * @param args
	 *            The arguments you want to launch Minecraft with. New path,
	 *            Username, Session ID.
	 */
	public static void main(String[] args)
	{
		if (args.length < 4)
		{
			System.out.println("Not enough arguments.");
			System.exit(-1);
		}
		
		Dimension winSize = new Dimension(854, 480);
		boolean maximize = false;
		boolean compatMode = false;
		
		if (args.length >= 5)
		{
			String[] dimStrings = args[4].split("x");
			
			if (args[4].equalsIgnoreCase("compatmode"))
			{
				compatMode = true;
			}
			else if (args[4].equalsIgnoreCase("max"))
			{
				maximize = true;
			}
			else if (dimStrings.length == 2)
			{
				try
				{
					winSize = new Dimension(Integer.parseInt(dimStrings[0]),
							Integer.parseInt(dimStrings[1]));
				}
				catch (NumberFormatException e)
				{
					System.out.println("Invalid Window size argument, " +
							"using default.");
				}
			}
			else
			{
				System.out.println("Invalid Window size argument, " +
						"using default.");
			}
		}
		
		try
		{
			System.out.println("Loading jars...");
			String[] jarFiles = new String[] {
				"minecraft.jar", "lwjgl.jar", "lwjgl_util.jar", "jinput.jar"
			};
			
			URL[] urls = new URL[jarFiles.length];
			
			for (int i = 0; i < urls.length; i++)
			{
				try
				{
					File f = new File(new File(args[0], "bin"), jarFiles[i]);
					urls[i] = f.toURI().toURL();
					System.out.println("Loading URL: " + urls[i].toString());
				} catch (MalformedURLException e)
				{
//					e.printStackTrace();
					System.err.println("MalformedURLException, " + e.toString());
					System.exit(5);
				}
			}
			
			System.out.println("Loading natives...");
			String nativesDir = new File(new File(args[0], "bin"), "natives").toString();
			
			System.setProperty("org.lwjgl.librarypath", nativesDir);
			System.setProperty("net.java.games.input.librarypath", nativesDir);

			System.setProperty("user.home", new File(args[0]).getParent());

			URLClassLoader cl = 
					new URLClassLoader(urls, MultiMCLauncher.class.getClassLoader());
			
			// Get the Minecraft Class.
			Class<?> mc = cl.loadClass("net.minecraft.client.Minecraft");
			Field[] fields = mc.getDeclaredFields();
			
			for (int i = 0; i < fields.length; i++)
			{
				Field f = fields[i];
				if (f.getType() != File.class)
				{
					// Has to be File
					continue;
				}
				if (f.getModifiers() != (Modifier.PRIVATE + Modifier.STATIC))
				{
					// And Private Static.
					continue;
				}
				f.setAccessible(true);
				f.set(null, new File(args[0]));
				// And set it.
				System.out.println("Fixed Minecraft Path: Field was " + f.toString());
			}
			
			String[] mcArgs = new String[2];
			mcArgs[0] = args[1];
			mcArgs[1] = args[2];

			String mcDir = 	mc.getMethod("a", String.class).invoke(null, (Object) "minecraft").toString();

			System.out.println("MCDIR: " + mcDir);
			
			if (compatMode)
			{
				System.out.println("Launching in compatibility mode...");
				mc.getMethod("main", String[].class).invoke(null, (Object) mcArgs);
			}
			else
			{
				System.out.println("Launching with applet wrapper...");
				try
				{
					Class<?> MCAppletClass = cl.loadClass(
							"net.minecraft.client.MinecraftApplet");
					Applet mcappl = (Applet) MCAppletClass.newInstance();
					MCFrame mcWindow = new MCFrame(args[3]);
					mcWindow.start(mcappl, args[1], args[2], winSize, maximize);
				} catch (InstantiationException e)
				{
					System.out.println("Applet wrapper failed! Falling back " +
							"to compatibility mode.");
					mc.getMethod("main", String[].class).invoke(null, (Object) mcArgs);
				}
			}
		} catch (ClassNotFoundException e)
		{
			e.printStackTrace();
			System.exit(1);
		} catch (IllegalArgumentException e)
		{
			e.printStackTrace();
			System.exit(2);
		} catch (IllegalAccessException e)
		{
			e.printStackTrace();
			System.exit(2);
		} catch (InvocationTargetException e)
		{
			e.printStackTrace();
			System.exit(3);
		} catch (NoSuchMethodException e)
		{
			e.printStackTrace();
			System.exit(3);
		} catch (SecurityException e)
		{
			e.printStackTrace();
			System.exit(4);
		}
	}
	
}
