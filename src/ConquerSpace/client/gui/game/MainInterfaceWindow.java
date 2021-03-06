/*
 * Conquer Space - Conquer Space!
 * Copyright (C) 2019 EhWhoAmI
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
package ConquerSpace.client.gui.game;

import static ConquerSpace.ConquerSpace.LOCALE_MESSAGES;
import ConquerSpace.client.gui.game.engineering.BuildSpaceShipAutomationMenu;
import ConquerSpace.client.gui.game.engineering.FullShipClassInformationMenu;
import ConquerSpace.client.gui.game.engineering.HullCreator;
import ConquerSpace.client.gui.game.engineering.ProductionProcessViewer;
import ConquerSpace.client.gui.game.engineering.shipcomponent.ShipComponentDesigner;
import ConquerSpace.client.gui.game.planetdisplayer.PlanetInfoSheet;
import ConquerSpace.client.gui.game.planetdisplayer.ShrinkedPlanetSheet;
import ConquerSpace.client.gui.game.planetdisplayer.UnownedPlanetInfoMenu;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.game.events.Event;
import ConquerSpace.common.game.organizations.Civilization;
import ConquerSpace.common.game.universe.bodies.Galaxy;
import ConquerSpace.common.game.universe.bodies.Planet;
import ConquerSpace.common.util.ResourceLoader;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.ImageIcon;
import javax.swing.JInternalFrame;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JTree;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 *
 * @author EhwhoAmI
 */
public class MainInterfaceWindow extends JInternalFrame implements MouseListener, InternalFrameListener {

    //Hold all the panels
    private JTabbedPane tabs;

    private ResearchViewer researchViewer;

    private JPanel planetInfoSheetContainer;
    private PlanetInfoSheet planetInfoSheet;
    private ShrinkedPlanetSheet shrinkedPlanetSheet;
    private UnownedPlanetInfoMenu unownedPlanetInfoMenu;
    private SpaceShipOverview spaceShipOverview;
    private HullCreator hullCreator;
    private EconomyWindow economyWindow;

    JTabbedPane shipsComponentsOverviewPanel;
    private BuildSpaceShipAutomationMenu buildSpaceShipAutomationMenu;
    private FullShipClassInformationMenu fullShipClassInformationMenu;
    private ProductionProcessViewer productionProcessViewer;

    private CivInfoOverview civInfoOverview;

    private PersonWindow personWindow;
    private RecruitingPerson recruitingPerson;

    private EventViewer eventViewer;

    private OrganizationsOrganizer organizationsOrganizer;

    private ResourceManager resourceManager;

    private Civilization civilization;
    private Galaxy universe;
    private GameState gameState;

    private JTree universeBreakdown;
    private DefaultMutableTreeNode universeBreakdownTreeModel;

    private Planet selectedPlanet = null;

    private final int planetinfotab = 1;

    private PlayerRegister register;

    public MainInterfaceWindow(GameState gameState, Civilization c, PlayerRegister register) {
        this.civilization = c;
        this.universe = gameState.getUniverse();
        this.gameState = gameState;
        this.register = register;

        init();

        //Set selected planet
        setSelectedPlanet(c.getCapitalPlanet(), true);
        setSelectedTab(0);

        update();
        expandAllNodes(universeBreakdown, 0, universeBreakdown.getRowCount());

        setClosable(true);
        setMaximizable(true);
        setResizable(true);
        setVisible(false);

        pack();
        Dimension d = getToolkit().getScreenSize();
        setSize(d.width - 100, d.height - 100);
        setDefaultCloseOperation(HIDE_ON_CLOSE);
        setTitle(LOCALE_MESSAGES.getMessage("game.mainwindow.title"));
    }

    public void init() {
        setLayout(new BorderLayout());
        tabs = new JTabbedPane();

        universeBreakdownTreeModel = new DefaultMutableTreeNode(LOCALE_MESSAGES.getMessage("game.breakdown.ownedstarsystems"));
        universeBreakdown = new JTree(universeBreakdownTreeModel);

        universeBreakdown.addTreeSelectionListener(new TreeSelectionListener() {
            @Override
            public void valueChanged(TreeSelectionEvent arg0) {
                DefaultMutableTreeNode selectedNode
                        = (DefaultMutableTreeNode) universeBreakdown.getLastSelectedPathComponent();
                if (selectedNode.getChildCount() == 0 && selectedNode.getUserObject() instanceof Planet) {
                    //Do stuff
                    Planet p = (Planet) selectedNode.getUserObject();
                    //Selected planet
                    setSelectedPlanet(p, p.hasScanned(civilization.getReference()));
                }
                //process
            }
        });

        researchViewer = new ResearchViewer(gameState, civilization);

        planetInfoSheetContainer = new JPanel();
        planetInfoSheetContainer.setLayout(new BorderLayout());

        //Space ship tabs
        spaceShipOverview = new SpaceShipOverview(gameState, civilization);

        //Engineering tabs
        JPanel shipComponentsOverview = new JPanel(new BorderLayout());

        shipsComponentsOverviewPanel = new JTabbedPane();

        buildSpaceShipAutomationMenu = new BuildSpaceShipAutomationMenu(gameState, civilization);
        shipsComponentsOverviewPanel.add(LOCALE_MESSAGES.getMessage("game.mainwindow.engineering.tabs.shipdesign"), buildSpaceShipAutomationMenu);

        fullShipClassInformationMenu = new FullShipClassInformationMenu(gameState, civilization);
        shipsComponentsOverviewPanel.add("Ship Classes", fullShipClassInformationMenu);

        ShipComponentDesigner shipComponentDesigner = new ShipComponentDesigner(gameState, civilization);
        shipsComponentsOverviewPanel.add(LOCALE_MESSAGES.getMessage("game.mainwindow.engineering.tabs.shipcomponent"), shipComponentDesigner);

        hullCreator = new HullCreator(gameState, civilization);
        shipsComponentsOverviewPanel.add(LOCALE_MESSAGES.getMessage("game.mainwindow.engineering.tabs.hulls"), hullCreator);

        productionProcessViewer = new ProductionProcessViewer(gameState, civilization);
        shipsComponentsOverviewPanel.add("Production process viewer", productionProcessViewer);

        shipComponentsOverview.add(shipsComponentsOverviewPanel, BorderLayout.CENTER);

        JTabbedPane peopleTabs = new JTabbedPane();
        personWindow = new PersonWindow(gameState, civilization);
        peopleTabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.people.list"), personWindow);

        recruitingPerson = new RecruitingPerson(gameState, civilization);
        peopleTabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.people.recruitment"), recruitingPerson);

        civInfoOverview = new CivInfoOverview(gameState, civilization);

        resourceManager = new ResourceManager(civilization);

        economyWindow = new EconomyWindow(civilization, universe);

        eventViewer = new EventViewer();

        organizationsOrganizer = new OrganizationsOrganizer(gameState, civilization);

        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.research"), researchViewer);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.planet"), planetInfoSheetContainer);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.ships"), spaceShipOverview);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.engineering"), shipComponentsOverview);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.people"), peopleTabs);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.civilization"), civInfoOverview);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.resources"), resourceManager);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.economy"), economyWindow);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.events"), eventViewer);
        tabs.add(LOCALE_MESSAGES.getMessage("game.mainwindow.tabs.orgs"), organizationsOrganizer);

        initTabIcons();

        add(universeBreakdown, BorderLayout.WEST);
        add(tabs, BorderLayout.CENTER);

        addListeners();

        updateComponents();
    }

    private void initTabIcons() {
        ImageIcon tab1Icon = ResourceLoader.getIcon("science.icon");
        ImageIcon econ = ResourceLoader.getIcon("economy.icon");
        ImageIcon engineering = ResourceLoader.getIcon("engineering.icon");
        ImageIcon spaceshps = ResourceLoader.getIcon("spaceships.icon");
        ImageIcon planet = ResourceLoader.getIcon("eclipse.icon");
        ImageIcon person = ResourceLoader.getIcon("person.icon");
        ImageIcon civ = ResourceLoader.getIcon("people.icon");
        ImageIcon goods = ResourceLoader.getIcon("goods.icon");
        ImageIcon events = ResourceLoader.getIcon("alert.icon");
        ImageIcon orgs = ResourceLoader.getIcon("org.icon");

        tabs.setIconAt(0, tab1Icon);
        tabs.setIconAt(1, planet);
        tabs.setIconAt(2, spaceshps);
        tabs.setIconAt(3, engineering);
        tabs.setIconAt(4, person);
        tabs.setIconAt(5, civ);
        tabs.setIconAt(6, goods);
        tabs.setIconAt(7, econ);
        tabs.setIconAt(8, events);
        tabs.setIconAt(9, orgs);
    }

    private void addListeners() {
        addMouseListener(new MouseAdapter() {
            @Override
            public void mouseEntered(MouseEvent e) {
                update();
            }
        });

        addComponentListener(new ComponentAdapter() {
            @Override
            public void componentShown(ComponentEvent e) {
                updateComponents();
            }
        });

        addMouseListener(new MouseAdapter() {
            @Override
            public void mouseEntered(MouseEvent e) {
                updateComponents();
            }
        });

        addPropertyChangeListener(l -> {
            updateComponents();
        });
    }

    public void update() {
        //Add the stuff
        universeBreakdownTreeModel.removeAllChildren();
        //get owned star systems

        for (ObjectReference p : civilization.getHabitatedPlanets()) {
            Planet planet = gameState.getObject(p, Planet.class);
            DefaultMutableTreeNode system = new DefaultMutableTreeNode(Integer.toString(planet.getParentIndex()));
            DefaultMutableTreeNode dm = new DefaultMutableTreeNode(planet);
            system.add(dm);
            universeBreakdownTreeModel.add(system);
        }

        if (spaceShipOverview.isVisible()) {
            spaceShipOverview.update();
        }
        if (recruitingPerson.isVisible()) {
            recruitingPerson.update();
        }
        if (personWindow.isVisible()) {
            personWindow.update();
        }
        if (researchViewer.isVisible()) {
            researchViewer.update();
        }
        if (resourceManager.isVisible()) {
            resourceManager.update();
        }
        if (economyWindow.isVisible()) {
            economyWindow.update();
        }

        if (planetInfoSheet != null) {
            planetInfoSheet.update();
        }
    }

    private void expandAllNodes(JTree tree, int startingIndex, int rowCount) {
        for (int i = startingIndex; i < rowCount; ++i) {
            tree.expandRow(i);
        }

        if (tree.getRowCount() != rowCount) {
            expandAllNodes(tree, rowCount, tree.getRowCount());
        }
    }

    public void setSelectedTab(int tab) {
        tabs.setSelectedIndex(tab);
    }

    public void setSelectedPlanet(Planet p, boolean toShowResources) {
        selectedPlanet = p;
        planetInfoSheetContainer.removeAll();
        if (toShowResources) {
            //Check if owned or not
            if (p.getOwnerReference().equals(civilization.getReference())) {
                planetInfoSheet = new PlanetInfoSheet(gameState, selectedPlanet, civilization, register);
                planetInfoSheetContainer.add(planetInfoSheet, BorderLayout.CENTER);
            } else {
                unownedPlanetInfoMenu = new UnownedPlanetInfoMenu(gameState, selectedPlanet, civilization, register);
                planetInfoSheetContainer.add(unownedPlanetInfoMenu, BorderLayout.CENTER);
            }
        } else {
            shrinkedPlanetSheet = new ShrinkedPlanetSheet(gameState, p, civilization, register);
            planetInfoSheetContainer.add(shrinkedPlanetSheet, BorderLayout.CENTER);
        }
        setSelectedTab(planetinfotab);
        setVisible(true);
    }

    public void passEvent(Event e) {
        //Add thing
        eventViewer.passEvent(e);
    }

    private void updateComponents() {
        /*shipsComponentsOverviewPanel.setEnabledAt(2, false);
        shipsComponentsOverviewPanel.setToolTipTextAt(2, "You need to research a launch system in the Research Tab!");
        if (civilization.values.containsKey("haslaunch") && civilization.values.get("haslaunch") == 1) {
            shipsComponentsOverviewPanel.setEnabledAt(2, true);
            shipsComponentsOverviewPanel.setToolTipText("");
        }*/
    }

    @Override
    public void internalFrameOpened(InternalFrameEvent e) {
        update();
    }

    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        //Leave empty
    }

    @Override
    public void internalFrameClosed(InternalFrameEvent e) {
        //Leave empty
    }

    @Override
    public void internalFrameIconified(InternalFrameEvent e) {
        //Leave empty
    }

    @Override
    public void internalFrameDeiconified(InternalFrameEvent e) {
        //Leave empty
    }

    @Override
    public void internalFrameActivated(InternalFrameEvent e) {
        update();
    }

    @Override
    public void internalFrameDeactivated(InternalFrameEvent e) {
        //Leave empty
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        //Leave empty
    }

    @Override
    public void mousePressed(MouseEvent e) {
        //Leave empty
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        //Leave empty
    }

    @Override
    public void mouseEntered(MouseEvent e) {
        update();
    }

    @Override
    public void mouseExited(MouseEvent e) {
        //Leave empty
    }
}
