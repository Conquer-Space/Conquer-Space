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
package ConquerSpace.common.game.organizations.civilization.government;

import ConquerSpace.common.game.characters.Person;
import java.io.Serializable;
import java.util.HashMap;

/**
 *
 * @author EhWhoAmI
 */
public class Government implements Serializable{
    public HashMap<GovernmentPosition, Person> officials;
    public PoliticalPowerSource politicalPowerSource;
    Person leader;
    String leaderTitle;
    
    //Important governenmt positions. All governments need these
    public GovernmentPosition headofState;
    public GovernmentPosition headofGovernment;

    public Government() {
        officials = new HashMap<>();
    }

    public Person getLeader() {
        return leader;
    }

    public void setLeader(Person leader) {
        this.leader = leader;
    }

    public String getLeaderTitle() {
        return leaderTitle;
    }

    public void setLeaderTitle(String leaderTitle) {
        this.leaderTitle = leaderTitle;
    }
}
