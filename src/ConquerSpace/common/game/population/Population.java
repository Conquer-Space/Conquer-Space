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
package ConquerSpace.common.game.population;

import ConquerSpace.common.ConquerSpaceGameObject;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.save.Serialize;
import ConquerSpace.common.save.SerializeClassName;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * Represents the population in a place.
 *
 * @author EhWhoAmI
 */
@SerializeClassName("population")
public class Population extends ConquerSpaceGameObject {

    @Serialize("segments")
    public final ArrayList<ObjectReference> segments;

    private long populationSize = 0;

    public Population(GameState gameState) {
        super(gameState);
        segments = new ArrayList<>();
    }

    public ObjectReference getSegment(int index) {
        return segments.get(index);
    }

    public void addSegment(ObjectReference seg) {
        segments.add(seg);
    }

    public long getPopulationSize() {
        populationSize = 0;
        for (ObjectReference seg : segments) {
            populationSize
                    += gameState.getObject(seg, PopulationSegment.class).getSize();
        }
        return populationSize;
    }

    public long getWorkableSize() {
        long populationSize = 0;
        for (ObjectReference seg : segments) {
            populationSize
                    += gameState.getObject(seg, PopulationSegment.class).workablePopulation;
        }
        return populationSize;
    }

    public Iterator<ObjectReference> getIterator() {
        return segments.iterator();
    }
}
