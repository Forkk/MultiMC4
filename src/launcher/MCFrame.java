import net.minecraft.Launcher;
import java.applet.Applet;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.net.MalformedURLException;
import java.net.URL;
import java.io.IOException;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;

public class MCFrame extends Frame implements WindowListener
{
	private Launcher appletWrap = null;
	
	public MCFrame(String title)
	{
		super(title);
		BufferedImage image = null;
		try
		{
			image = ImageIO.read(getClass().getResource("icon.png"));
			if(image.getTransparency() != java.awt.Transparency.TRANSLUCENT
			   || !image.getColorModel().hasAlpha())
			{
				System.err.println("Icon data is not translucent!");
			}
			setIconImage(image);
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		super.setVisible(true);
		
		this.setSize(800, 600);
		this.setLocationRelativeTo(null);
		
		this.setResizable(true);
		this.addWindowListener(this);
	}

	public void start(Applet mcApplet, String user, String session, 
			Dimension winSize, boolean maximize)
	{
		try
		{
			appletWrap = new Launcher(mcApplet, new URL("http://www.minecraft.net/game"));
		}
		catch (MalformedURLException ignored){}
		
		appletWrap.setParameter("username", user);
		appletWrap.setParameter("sessionid", session);
		appletWrap.setParameter("stand-alone", "true"); // Show the quit button.
		mcApplet.setStub(appletWrap);
		
		this.add(appletWrap);
		appletWrap.setPreferredSize(winSize);
		this.pack();
		this.setLocationRelativeTo(null);
		if (maximize)
			this.setExtendedState(MAXIMIZED_BOTH);
		
		validate();
		appletWrap.init();
		appletWrap.start();
		setVisible(true);
	}

	@Override
	public void windowActivated(WindowEvent e) {}

	@Override
	public void windowClosed(WindowEvent e) {}

	@Override
	public void windowClosing(WindowEvent e)
	{
		new Thread()
		{
			public void run()
			{
				try
				{
					Thread.sleep(30000L);
				} catch (InterruptedException localInterruptedException)
				{
					localInterruptedException.printStackTrace();
				}
				System.out.println("FORCING EXIT!");
				System.exit(0);
			}
		}
		.start();

		if (appletWrap != null)
		{
			appletWrap.stop();
			appletWrap.destroy();
		}
		// old minecraft versions can hang without this >_<
		System.exit(0);
	}

	@Override
	public void windowDeactivated(WindowEvent e) {}

	@Override
	public void windowDeiconified(WindowEvent e) {}

	@Override
	public void windowIconified(WindowEvent e) {}

	@Override
	public void windowOpened(WindowEvent e) {}
}