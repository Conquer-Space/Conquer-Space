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
package ConquerSpace.client.gui.game.engineering.shipcomponent;

import ConquerSpace.common.game.ships.components.ShipComponent;
import javax.swing.JPanel;

/**
 *
 * @author EhWhoAmI
 */
public abstract class ShipComponentDesignerPanel extends JPanel {

    ShipComponent generateComponent() {
        return null;
    }

    public abstract void clearUI();

    public void loadComponent(ShipComponent comp) {
        //Leave empty
    }

}
