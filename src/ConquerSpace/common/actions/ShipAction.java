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
package ConquerSpace.common.actions;

import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.game.ships.SpaceShip;

/**
 *
 * @author EhWhoAmI
 */
public abstract class ShipAction {

    protected ObjectReference ship;
    
    protected boolean done;
    

    public ShipAction(SpaceShip ship) {
        this.ship = ship.getReference();
    }

    /**
     * Step the action.
     * @param gameState  gameState
     */
    public abstract void doAction(GameState gameState);
    
    /**
     * Initializes the action.
     * @param gameState 
     */
    public abstract void initAction(GameState gameState);
    
    /**
     * Is this action possible?
     * @param gameState gamestate
     * @return if possible
     */
    public abstract boolean isPossible(GameState gameState);
    
    public boolean checkIfDone(GameState gameState) {
        return true;
    }
}
