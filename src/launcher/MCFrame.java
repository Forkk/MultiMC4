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

public class MCFrame extends Frame implements WindowListener
{
	private AppletWrap appletWrap = null;

	public MCFrame(String title, String icon_name)
	{
		super(title);
		//TODO: use instance icon here
		
		super.setVisible(true);
		
		//FIXME: this can fail in multiple monitor setups
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		this.setSize(800,600);
		this.setLocation((screenSize.width - 800) / 2, (screenSize.height - 600) / 2);
		
		this.setResizable(true);
		this.addWindowListener(this);
	}

	public void start(Applet mcApplet, String user, String session)
	{
		try
		{
			appletWrap = new AppletWrap(mcApplet, new URL("http://www.minecraft.net/game"));
		}
		catch (MalformedURLException ignored){}
		
		appletWrap.setParameter("username", user);
		appletWrap.setParameter("sessionid", session);
		mcApplet.setStub(appletWrap);

		this.add(appletWrap);
		validate();
		appletWrap.init();
		appletWrap.setSize(getWidth(), getHeight());
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