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

import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.game.population.jobs.JobType;
import ConquerSpace.common.save.SerializeClassName;

/**
 * Symbolizes all the smaller businesses and normal day to day trade in
 *
 * @author EhWhoAmI
 */
@SerializeClassName("commercial-area")
public class CommercialArea extends Area {

    private int tradeValue;
    private ObjectReference currency;

    CommercialArea(GameState gameState) {
        super(gameState);
        priority = 10;
    }

    public void setTradeValue(int tradeValue) {
        this.tradeValue = tradeValue;
    }

    public int getTradeValue() {
        return tradeValue;
    }

    @Override
    public String toString() {
        return "Commercial Area";
    }

    @Override
    public void accept(AreaDispatcher dispatcher) {
        dispatcher.dispatch(this);
    }

    @Override
    public JobType getJobClassification() {
        return JobType.Independent;
    }

    @Override
    public AreaClassification getAreaType() {
        return AreaClassification.Commercial;
    }

    public ObjectReference getCurrency() {
        return currency;
    }

    public void setCurrency(ObjectReference currency) {
        this.currency = currency;
    }
}
