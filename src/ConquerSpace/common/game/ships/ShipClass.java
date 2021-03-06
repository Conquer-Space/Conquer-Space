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
package ConquerSpace.common.game.ships;

import ConquerSpace.common.ConquerSpaceGameObject;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.save.SerializeClassName;
import java.io.Serializable;
import java.util.ArrayList;

/**
 *
 * @author EhWhoAmI
 */
@SerializeClassName("ship-class")
public class ShipClass extends ConquerSpaceGameObject implements Serializable {

    private static final long serialVersionUID = 1L;

    private String name;
    private ObjectReference hull;
    public ArrayList<ObjectReference> components;
    private int mass = 1;
    private long estimatedThrust = 0;
    public ArrayList<ShipCapability> capabilities;

    public ShipClass(GameState gameState, String name, ObjectReference h) {
        super(gameState);
        this.name = name;
        this.hull = h;
        components = new ArrayList<>();
        capabilities = new ArrayList<>();
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }

    public ObjectReference getHull() {
        return hull;
    }

    public void setHull(ObjectReference hull) {
        this.hull = hull;
    }

    public int getMass() {
        //Mass is hull + components and stuff
        return mass;
    }

    public long getEstimatedThrust() {
        return estimatedThrust;
    }

    public void setEstimatedThrust(long estimatedThrust) {
        this.estimatedThrust = estimatedThrust;
    }
}
