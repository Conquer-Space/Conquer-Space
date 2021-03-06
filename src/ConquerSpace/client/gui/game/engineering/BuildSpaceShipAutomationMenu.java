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
package ConquerSpace.client.gui.game.engineering;

import static ConquerSpace.ConquerSpace.LOCALE_MESSAGES;
import ConquerSpace.client.gui.ObjectListModel;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.game.organizations.Civilization;
import ConquerSpace.common.game.ships.Hull;
import ConquerSpace.common.game.ships.ShipCapability;
import ConquerSpace.common.game.ships.ShipClass;
import ConquerSpace.common.game.ships.ShipType;
import ConquerSpace.common.game.ships.components.ControlComponent;
import ConquerSpace.common.game.ships.components.ShipComponent;
import ConquerSpace.common.game.ships.components.ToOrbitEngineComponent;
import ConquerSpace.common.util.names.NameGenerator;
import com.alee.extended.layout.HorizontalFlowLayout;
import com.alee.extended.layout.VerticalFlowLayout;
import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.SpinnerNumberModel;

/**
 *
 * @author EhWhoAmI
 */
public class BuildSpaceShipAutomationMenu extends JPanel {

    private Civilization civ;

    private ArrayList<ObjectReference> objectComponentsToAdd;

    private JList<String> shipList;
    private ObjectListModel<ObjectReference> shipListModel;

    JPanel shipDetailsPanel;
    private JLabel shipNameLabel;
    private JTextField shipNameField;
    private JButton randomShipNameButton;

    private JLabel massLabel;
    private JLabel massText;
    private JLabel massUnit;

    private JLabel hullLabel;
    private JCheckBox toDesignHullOrNotToDesign;
    private JButton chooseHullButton;

    private JLabel hullNameLabel;
    private JLabel hullSpaceLabel;
    private JLabel hullMaterialLabel;

    private JLabel shipTypeLabel;
    private JComboBox<ShipType> shipTypeComboBox;

    private JLabel shipArmorLabel;

    private JLabel shipShieldLabel;

    private JLabel shipSpeedLabel;
    private JLabel shipSpeedUnit;
    private JSpinner shipSpeedSpinner;

    private JLabel engineTypeLabel;
    private JButton setEngineButton;
    private JLabel engineTypeIndicatorLabel;

    private JLabel toOrbitEngineLabel;
    private JButton toOrbitEngineCustomizeButton;
    private JCheckBox toOrbitEngineEnableButton;
    private ObjectReference toOrbitEngineType;
    //What the engine is
    private ObjectReference toOrbitEngineSelection;
    //private int toOrbitEngineAmount;
    private boolean designToOrbitEngineOrNot;

//    private JLabel fuelCapacityLabel;
//    private JLabel fuelCapacityValue;
//    private JLabel fuelCapacityUnit;
//    private JButton fuelCapicityConfig;

    private JLabel shipSensorsLabel;
    private JButton shipSensorButton;
    private JLabel shipScienceLabel;
    private JButton shipScienceButton;

    JList<String> shipComponents;
    JList<String> alreadyAddedShipComponentList;

    private final JTabbedPane mainTabs;
    private JPanel componentRoughDesignPanel;

    //Hull is null means it is self designed
    private Hull selectedHull = null;

    private NameGenerator nameGenerator;

    private GameState gameState;

    public BuildSpaceShipAutomationMenu(GameState gameState, Civilization c) {
        this.gameState = gameState;
        this.civ = c;
        setLayout(new BorderLayout());

        try {
            nameGenerator = NameGenerator.getNameGenerator("component.names");
        } catch (IOException ex) {
        }

        JToolBar menuBar = new JToolBar();
        JButton newShipClass = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.newclass"));
        JButton saveShipClass = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.design"));

        newShipClass.addActionListener(l -> {
            //Empty all components
            shipSpeedSpinner.setValue(0);
            shipNameField.setText("");

            selectedHull = null;
        });

        saveShipClass.addActionListener(l -> {
            createShip();
        });

        menuBar.add(newShipClass);
        menuBar.add(saveShipClass);
        add(menuBar, BorderLayout.NORTH);

        JPanel shipInformationPanel = new JPanel();

        shipInformationPanel.setLayout(new HorizontalFlowLayout());

        //The panel that you cant really change the stuff inside
        shipListModel = new ObjectListModel<>();
        shipListModel.setElements(civ.getShipClasses());

        shipListModel.setHandler(l -> {
            return gameState.getObject(l, ShipClass.class).getName();
        });

        shipList = new JList<>(shipListModel);
        shipList.setFixedCellWidth(250);
        JScrollPane shipListScrollPane = new JScrollPane(shipList);
        shipInformationPanel.add(shipListScrollPane);

        shipDetailsPanel = new JPanel();
        JPanel shipDetailsContainer = new JPanel(new VerticalFlowLayout());
        {
            shipDetailsPanel.setLayout(new GridBagLayout());
            shipNameLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.shipname"));
            shipNameField = new JTextField();
            shipNameField.setColumns(16);
            randomShipNameButton = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.randomname"));
            randomShipNameButton.addActionListener(l -> {
                shipNameField.setText(nameGenerator.getName(0));
            });

            massLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.mass"));
            massText = new JLabel("0");
            massUnit = new JLabel("kg");

            shipTypeLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.shiptype"));

            shipTypeComboBox = new JComboBox<>(Arrays.copyOf(gameState.shipTypes.toArray(), gameState.shipTypes.size(), ShipType[].class));
            shipTypeComboBox.addActionListener(l -> {
                ShipType shipType = (ShipType) shipTypeComboBox.getSelectedItem();

                shipScienceButton.setEnabled(false);
                shipSpeedSpinner.setEnabled(true);

                //Enable if it is science
                if (shipType.containsTag("science")) {
                    shipScienceButton.setEnabled(true);
                }

                if (shipType.containsTag("launch")) {
                    shipSpeedSpinner.setEnabled(false);
                }
            });

            shipSpeedLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.speed"));
            shipSpeedUnit = new JLabel("<html>m/s<sup>2</sup></html");
            shipSpeedSpinner = new JSpinner(new SpinnerNumberModel(10, 0, 1000000, 10));

            hullLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull"));
            toDesignHullOrNotToDesign = new JCheckBox(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull.autodesign"));
            toDesignHullOrNotToDesign.setSelected(true);
            toDesignHullOrNotToDesign.addActionListener(l -> {
                if (toDesignHullOrNotToDesign.isSelected()) {
                    chooseHullButton.setEnabled(false);
                } else {
                    chooseHullButton.setEnabled(true);
                }
            });
            chooseHullButton = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull.choose"));
            chooseHullButton.addActionListener(l -> {
                ShipHullConfigurePanel panel = new ShipHullConfigurePanel(gameState, civ);
                int close = JOptionPane.showInternalConfirmDialog(this,
                        panel, "Hull Chooser",
                        JOptionPane.OK_CANCEL_OPTION, JOptionPane.PLAIN_MESSAGE);
                if (close == JOptionPane.OK_OPTION) {
                    //Get selected hull
                    if (panel.getSelectedHull() != null) {
                        selectedHull = panel.getSelectedHull();
                    }
                }
            });
            chooseHullButton.setEnabled(false);

            hullNameLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull.autodesign.name"));
            hullSpaceLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull.autodesign.volume"));
            hullMaterialLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.hull.autodesign.material"));

            GridBagConstraints constraints = new GridBagConstraints();
            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;

            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipNameLabel, constraints);
            constraints.gridx = 1;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;

            shipDetailsPanel.add(shipNameField, constraints);
            constraints.gridx = 2;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;

            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(randomShipNameButton, constraints);

            constraints.gridx = 0;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;

            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(massLabel, constraints);
            constraints.gridx = 1;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(massText, constraints);
            constraints.gridx = 2;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(massUnit, constraints);

            constraints.gridx = 0;
            constraints.gridy = 2;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipTypeLabel, constraints);
            constraints.gridx = 1;
            constraints.gridy = 2;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipTypeComboBox, constraints);

            constraints.gridx = 0;
            constraints.gridy = 3;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipSpeedLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 3;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipSpeedSpinner, constraints);
            constraints.gridx = 2;
            constraints.gridy = 3;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(shipSpeedUnit, constraints);
            constraints.gridx = 0;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(hullLabel, constraints);
            constraints.gridx = 1;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(toDesignHullOrNotToDesign, constraints);
            constraints.gridx = 2;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(chooseHullButton, constraints);
            constraints.gridx = 0;
            constraints.gridy = 5;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            constraints.gridwidth = GridBagConstraints.REMAINDER;
            shipDetailsPanel.add(hullNameLabel, constraints);

            constraints.gridx = 0;
            constraints.gridy = 6;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            constraints.gridwidth = GridBagConstraints.REMAINDER;
            shipDetailsPanel.add(hullSpaceLabel, constraints);

            constraints.gridx = 0;
            constraints.gridy = 7;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            constraints.gridwidth = GridBagConstraints.REMAINDER;
            constraints.fill = GridBagConstraints.HORIZONTAL;
            shipDetailsPanel.add(hullMaterialLabel, constraints);
            shipDetailsContainer.add(shipDetailsPanel);
        }

        JPanel shipChangablePanel = new JPanel(new GridBagLayout());
        JPanel shipChangablePanelContainer = new JPanel(new GridBagLayout());
        {
            GridBagConstraints constraints = new GridBagConstraints();

            shipArmorLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.armor.label"));

            shipShieldLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.shields.label"));

            engineTypeLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.engines.label"));
            setEngineButton = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.engines.button"));
            setEngineButton.addActionListener(l -> {
                ShipEngineConfigurePanel panel = new ShipEngineConfigurePanel(gameState, civ);
                int close = JOptionPane.showInternalConfirmDialog(this,
                        panel, "Engine Chooser",
                        JOptionPane.OK_CANCEL_OPTION, JOptionPane.PLAIN_MESSAGE);
                if (close == JOptionPane.OK_OPTION) {
                    //Then select engine
                    ObjectReference engine = panel.selectedEngine;
                }
            });
            engineTypeIndicatorLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.engines.none"));

            toOrbitEngineLabel = new JLabel("Orbital rated engine:");
            toOrbitEngineCustomizeButton = new JButton("Customize engine");
            toOrbitEngineCustomizeButton.addActionListener(l -> {
                ShipToOrbitEngineConfigurePanel panel = new ShipToOrbitEngineConfigurePanel(gameState, civ);
                int close = JOptionPane.showInternalConfirmDialog(this,
                        panel, "Customize Orbital Engine",
                        JOptionPane.OK_CANCEL_OPTION, JOptionPane.PLAIN_MESSAGE);
                if (close == JOptionPane.OK_OPTION) {
                    designToOrbitEngineOrNot = panel.getToDesign();
                    if (designToOrbitEngineOrNot) {
                        //If designing
                        //toOrbitEngineAmount = panel.getAmountToOrbit();
                        toOrbitEngineType = panel.getEngineTech();
                    } else {
                        toOrbitEngineSelection = panel.getShipComponent();
                    }
                }
            });

            toOrbitEngineEnableButton = new JCheckBox("Enable orbital capability");
            toOrbitEngineEnableButton.setSelected(true);
            toOrbitEngineEnableButton.addActionListener(l -> {
                toOrbitEngineCustomizeButton.setEnabled(toOrbitEngineEnableButton.isSelected());
            });
//            fuelCapacityLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.fuel.label"));
//            fuelCapacityValue = new JLabel("0");
//            fuelCapacityUnit = new JLabel("<html>m<sup>3</sup></html");
//            fuelCapicityConfig = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.fuel.configure"));

            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(shipArmorLabel, constraints);
            constraints.gridx = 1;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.nocombat")), constraints);

            constraints.gridx = 0;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(shipShieldLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.nocombat")), constraints);

            constraints.gridx = 0;
            constraints.gridy = 2;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(engineTypeLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 2;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 0;
            //constraints.weighty = 1;
            shipChangablePanel.add(setEngineButton, constraints);

            constraints.gridx = 2;
            constraints.gridy = 2;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 1;
            //constraints.weighty = 1;
            shipChangablePanel.add(engineTypeIndicatorLabel, constraints);

            constraints.gridx = 0;
            constraints.gridy = 3;
            shipChangablePanel.add(toOrbitEngineLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 3;
            shipChangablePanel.add(toOrbitEngineCustomizeButton, constraints);

            constraints.gridx = 2;
            constraints.gridy = 3;
            shipChangablePanel.add(toOrbitEngineEnableButton, constraints);

            constraints.gridx = 0;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 1;
            //constraints.weighty = 1;
            //shipChangablePanel.add(fuelCapacityLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 1;
            //constraints.weighty = 1;
            //shipChangablePanel.add(fuelCapacityValue, constraints);

            constraints.gridx = 2;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 1;
            //constraints.weighty = 1;
            //shipChangablePanel.add(fuelCapacityUnit, constraints);

            constraints.gridx = 3;
            constraints.gridy = 4;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            //constraints.weightx = 1;
            //constraints.weighty = 1;
            //shipChangablePanel.add(fuelCapicityConfig, constraints);
            
            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            constraints.weightx = 1;
            constraints.weighty = 1;
            shipChangablePanelContainer.add(shipChangablePanel, constraints);
        }

        componentRoughDesignPanel = new JPanel();
        //Format stuff

        {
            componentRoughDesignPanel.setLayout(new GridBagLayout());
            JPanel grandContainer = new JPanel(new GridBagLayout());

            shipSensorsLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.designer.sensors"));
            shipSensorButton = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.designer.sensors.configure"));
            shipSensorButton.addActionListener(l -> {
                JOptionPane.showInternalMessageDialog(BuildSpaceShipAutomationMenu.this, "Not implemented yet");
            });

            shipScienceLabel = new JLabel(LOCALE_MESSAGES.getMessage("game.engineering.ship.designer.science"));
            shipScienceButton = new JButton(LOCALE_MESSAGES.getMessage("game.engineering.ship.designer.science.configure"));
            shipScienceButton.addActionListener(l -> {
                JOptionPane.showInternalMessageDialog(BuildSpaceShipAutomationMenu.this, "Not implemented yet");
            });
            shipScienceButton.setEnabled(false);

            //Another panel to add random ship components
            JPanel addRandomShipComponentPanel = new JPanel();
            ObjectListModel<ObjectReference> shipComponentListModel = new ObjectListModel<>();
            shipComponentListModel.setElements(c.getShipComponentList());
            shipComponentListModel.setHandler(l -> {
                return gameState.getObject(l, ShipComponent.class).getName();
            });

            objectComponentsToAdd = new ArrayList<>();

            shipComponents = new JList<>(shipComponentListModel);
            shipComponents.setFixedCellWidth(64);

            ObjectListModel<ObjectReference> addedShipComponentListModel = new ObjectListModel<>();
            addedShipComponentListModel.setElements(objectComponentsToAdd);
            addedShipComponentListModel.setHandler(l -> {
                return gameState.getObject(l, ShipComponent.class).getName();
            });

            alreadyAddedShipComponentList = new JList<>(addedShipComponentListModel);
            alreadyAddedShipComponentList.setFixedCellWidth(64);

            //Add button to add component
            JButton addSelectedComponentButton = new JButton("Add Component");
            addSelectedComponentButton.addActionListener(l -> {
                int selected = shipComponents.getSelectedIndex();
                if (selected >= 0) {
                    //Then get the component and add it to the thing
                    ObjectReference reference = shipComponentListModel.getObject(selected);
                    objectComponentsToAdd.add(reference);
                    alreadyAddedShipComponentList.updateUI();
                }
            });

            addRandomShipComponentPanel.add(new JScrollPane(shipComponents));
            addRandomShipComponentPanel.add(addSelectedComponentButton);
            addRandomShipComponentPanel.add(new JScrollPane(alreadyAddedShipComponentList));

            GridBagConstraints constraints = new GridBagConstraints();

            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            grandContainer.add(shipSensorsLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            grandContainer.add(shipSensorButton, constraints);

            constraints.gridx = 0;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            grandContainer.add(shipScienceLabel, constraints);

            constraints.gridx = 1;
            constraints.gridy = 1;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            grandContainer.add(shipScienceButton, constraints);

            constraints.gridx = 2;
            constraints.gridy = 0;
            grandContainer.add(addRandomShipComponentPanel, constraints);

            //So that it goes to the top left corner...            
            constraints.gridx = 0;
            constraints.gridy = 0;
            constraints.anchor = GridBagConstraints.NORTHWEST;
            constraints.weightx = 1;
            constraints.weighty = 1;
            componentRoughDesignPanel.add(grandContainer, constraints);
        }

        //Init all final stuff to init. 
        {
            ShipType shipType = (ShipType) shipTypeComboBox.getSelectedItem();
            shipScienceButton.setEnabled(false);

            //Enable if it is science
            if (shipType.containsTag("science")) {
                shipScienceButton.setEnabled(true);
            }
        }
        mainTabs = new JTabbedPane(JTabbedPane.TOP, JTabbedPane.SCROLL_TAB_LAYOUT);

        shipInformationPanel.add(shipDetailsContainer);

        shipInformationPanel.add(shipChangablePanelContainer);

        mainTabs.add(LOCALE_MESSAGES.getMessage("game.engineering.ship.info"), shipInformationPanel);
        mainTabs.add(LOCALE_MESSAGES.getMessage("game.engineering.ship.components"), componentRoughDesignPanel);
        add(mainTabs, BorderLayout.CENTER);
    }

    /**
     * So that we can update the UI every time it becomes visible
     *
     * @param aFlag
     */
    @Override
    public void setVisible(boolean aFlag) {
        super.setVisible(aFlag);
        //Update stuff
        if (aFlag) {
            shipComponents.updateUI();
            alreadyAddedShipComponentList.updateUI();
            shipList.updateUI();
        }
    }

    public void createShip() {
        //Create ship class...
        String shipName = shipNameField.getText();
        if (selectedHull == null) {
            //Autogenerate...
            //Need to make hull material...
            //Get hull material
            selectedHull = new Hull(gameState, 100, 100,
                    civ.getHullMaterials().get(0),
                    (ShipType) shipTypeComboBox.getSelectedItem(), 100, shipName + " Hull");
            civ.getHulls().add(selectedHull.getReference());
        }

        //Add hull
        ShipClass sc = new ShipClass(gameState, shipName, selectedHull.getReference());
        //Add components
        sc.components.addAll(objectComponentsToAdd);

        //Add control component
        ControlComponent component = new ControlComponent(gameState);
        component.setName(shipName + " controller");
        sc.components.add(component.getReference());

        //To orbit capabilities
        if (toOrbitEngineEnableButton.isSelected()) {
            sc.capabilities.add(ShipCapability.ToOrbit);
            //Add thing
            if (designToOrbitEngineOrNot) {
                //Or else design engine
                //F/m=a
                double thrust = sc.getMass() * Double.parseDouble(gameState.constants.getProperty("toorbit.acceleration"));
                //Create component
                ToOrbitEngineComponent engineComponent = new ToOrbitEngineComponent(gameState);
                engineComponent.setThrust((int) thrust);
                engineComponent.setLaunchSystemType(toOrbitEngineType);
                engineComponent.setName("Launch system");
                sc.components.add(engineComponent.getReference());
                civ.getShipComponentList().add(engineComponent.getReference());
            } else if (toOrbitEngineSelection != null) {
                sc.components.add(toOrbitEngineSelection);
            }

        }

        //And other components
        //Autogenerate engine, etc...
        civ.getShipClasses().add(sc.getReference());
        shipList.updateUI();

        //Reset the UI
        selectedHull = null;
    }
}
