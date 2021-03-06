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
package ConquerSpace.common.game.universe;

import ConquerSpace.common.game.universe.bodies.Body;
import ConquerSpace.common.save.Serialize;
import java.io.Serializable;

/**
 * Orbit description
 *
 * @author EhWhoAmI
 */
public class Orbit implements Serializable {

    /**
     * The degrees
     */
    @Serialize("degrees")
    public double degrees;

    @Serialize("sma")
    public double semiMajorAxis;

    @Serialize("eccentricity")
    public double eccentricity;

    /**
     * The tilt of the orbit, in degrees.
     */
    @Serialize("rotation")
    public double rotation;

    public Body referenceBody;

    public Orbit(double degrees, double semiMajorAxis, double eccentricity, double rotation) {
        this.degrees = degrees;
        this.semiMajorAxis = semiMajorAxis;
        this.eccentricity = eccentricity;
        this.rotation = rotation;
    }

    public PolarCoordinate toPolarCoordinate() {
        double theta = Math.toRadians(this.degrees);
        double a = semiMajorAxis;

        double rotation = Math.toRadians(this.rotation);
        double r = (a * (1 - eccentricity * eccentricity)) / (1 - eccentricity * Math.cos(theta - rotation));
        return new PolarCoordinate(this.degrees, r);
    }

    public SpacePoint toSpacePoint() {
        return toPolarCoordinate().toSpacePoint();
    }

    public double getDegrees() {
        return degrees;
    }

    public double getEccentricity() {
        return eccentricity;
    }

    public double getRotation() {
        return rotation;
    }

    public double getSemiMajorAxis() {
        return semiMajorAxis;
    }

    public void setDegrees(double degrees) {
        this.degrees = degrees;
    }

    public void setEccentricity(double eccentricity) {
        this.eccentricity = eccentricity;
    }

    public void setRotation(double rotation) {
        this.rotation = rotation;
    }

    public void setSemiMajorAxis(double semiMajorAxis) {
        this.semiMajorAxis = semiMajorAxis;
    }

    public void modDegrees(float degs) {
        degrees += degs;
        degrees %= 360;
    }

    /**
     * Farthest point from the center of orbit
     *
     * @return
     */
    public double getApoapsis() {
        return eccentricity * semiMajorAxis + semiMajorAxis;
    }

    /**
     * Closest point from center of orbit
     *
     * @return
     */
    public double getPeriapsis() {
        return semiMajorAxis - eccentricity * semiMajorAxis;
    }
}
