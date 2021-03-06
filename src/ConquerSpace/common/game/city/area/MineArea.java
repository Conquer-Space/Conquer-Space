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
package ConquerSpace.common.game.city.area;

import ConquerSpace.common.ConstantStarDate;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.game.population.jobs.JobType;
import ConquerSpace.common.game.resources.ResourceStockpile;
import ConquerSpace.common.game.resources.StoreableReference;
import ConquerSpace.common.save.SerializeClassName;
import java.util.HashMap;

/**
 *
 * @author EhWhoAmI
 */
@SerializeClassName("mine-area")
public class MineArea extends Area implements ResourceStockpile {

    private ObjectReference miningStratum;
    private float productivity;
    private HashMap<StoreableReference, Double> necessaryGoods;
    private StoreableReference resourceMined;

    /**
     * Last date where this thing mined. Used to calculate resources.
     */
    private ConstantStarDate lastStarDateRemoved;

    MineArea(GameState gameState, ObjectReference mining, StoreableReference resourceMined, float productivity) {
        super(gameState);
        this.miningStratum = mining;
        this.productivity = productivity;
        this.resourceMined = resourceMined;
        necessaryGoods = new HashMap<>();
        lastStarDateRemoved = new ConstantStarDate(0);
    }

    @Override
    public AreaClassification getAreaType() {
        return AreaClassification.Mine;
    }

    public float getProductivity() {
        return productivity;
    }

    public ObjectReference getStratumMining() {
        return miningStratum;
    }

    public HashMap<StoreableReference, Double> getNecessaryGoods() {
        return necessaryGoods;
    }

    public StoreableReference getResourceMinedId() {
        return resourceMined;
    }

    @Override
    public String toString() {
        return "Mine";
    }

    @Override
    public JobType getJobClassification() {
        return (JobType.Miner);
    }

    @Override
    public void accept(AreaDispatcher dispatcher) {
        //Get last time 
        dispatcher.dispatch(this);
    }

    @Override
    public void addResourceTypeStore(StoreableReference type) {
    }

    @Override
    public Double getResourceAmount(StoreableReference type) {
        if (type == resourceMined) {
            return Double.valueOf(productivity) * (double) (gameState.date.getDate() - lastStarDateRemoved.getDate());
        }
        return 0d;
    }

    @Override
    public void addResource(StoreableReference type, Double amount) {
        //Void
    }

    @Override
    public boolean canStore(StoreableReference type) {
        return true;
    }

    @Override
    public boolean hasResource(StoreableReference type) {
        return (type == resourceMined);
    }

    @Override
    public StoreableReference[] storedTypes() {
        return new StoreableReference[]{resourceMined};
    }

    @Override
    public boolean removeResource(StoreableReference type, Double amount) {
        if (type == resourceMined && amount <= getResourceAmount(type)) {
            //Reset date
            lastStarDateRemoved = gameState.date.getConstantDate();
            return true;
        }
        return false;
    }

    public ConstantStarDate getLastStarDateRemoved() {
        return lastStarDateRemoved;
    }

    public void setLastStarDateRemoved(ConstantStarDate lastStarDateRemoved) {
        this.lastStarDateRemoved = lastStarDateRemoved;
    }

    @Override
    public void preResourceTransfer(StoreableReference type, Double amount, ResourceStockpile toWhere) {
        //Do nothing
    }

    @Override
    public void postResourceTransfer(StoreableReference type, Double amount, ResourceStockpile toWhere) {
        //Do nothing
    }
}
