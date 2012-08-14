import java.util.TreeMap;
import java.util.Map;
import java.net.URL;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.applet.Applet;
import java.applet.AppletStub;

public class AppletWrap extends Applet implements AppletStub
{
	private Applet wrappedApplet;
	private URL documentBase;
	private boolean active = false;
	private final Map<String, String> params;

	public AppletWrap(Applet wrappedApplet, URL documentBase)
	{
		params = new TreeMap<String, String>();
		setLayout(new GridBagLayout());
		java.awt.GridBagConstraints layoutParams =
			new java.awt.GridBagConstraints(0,0, // gridx,y
											1,1, // grid size x,y
											1.0,1.0, // weight
											java.awt.GridBagConstraints.FIRST_LINE_START, // anchor
											java.awt.GridBagConstraints.BOTH, // fill
											new java.awt.Insets(0, 0, 0, 0), // insets
											0,0); // pad
		this.add(wrappedApplet, layoutParams);
		this.wrappedApplet = wrappedApplet;
		this.documentBase = documentBase;
	}

	public void setParameter(String name, String value)
	{
		params.put(name, value);
	}

	@Override
	public String getParameter(String name)
	{
		String param = params.get(name);
		if (param != null)
			return param;
		try
		{
			return super.getParameter(name);
		} catch (Exception ignore){}
		return null;
	}

	@Override
	public boolean isActive()
	{
		return active;
	}

	@Override
	public void appletResize(int width, int height)
	{
		wrappedApplet.resize(width, height);
	}

	@Override
	public void resize(int width, int height)
	{
		wrappedApplet.resize(width, height);
	}

	@Override
	public void resize(Dimension d)
	{
		wrappedApplet.resize(d);
	}
	
	@Override
	public void init()
	{
		if (wrappedApplet != null)
		{
			wrappedApplet.init();
		}
	}

	@Override
	public void start()
	{
		wrappedApplet.start();
		active = true;
	}

	@Override
	public void stop()
	{
		wrappedApplet.stop();
		active = false;
	}

	@Override
	public URL getCodeBase() {
		return wrappedApplet.getCodeBase();
	}

	@Override
	public URL getDocumentBase()
	{
		return documentBase;
	}

	@Override
	public void setVisible(boolean b)
	{
		super.setVisible(b);
		wrappedApplet.setVisible(b);
	}
}