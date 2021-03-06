/*
 * jNoiseLib [https://github.com/andrewgp/jLibNoise]
 * Original code from libnoise [https://github.com/andrewgp/jLibNoise]
 *
 * Copyright (C) 2003, 2004 Jason Bevins
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License (COPYING.txt) for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * The developer's email is jlbezigvins@gmzigail.com (for great email, take
 * off every 'zig'.)
 */
package ConquerSpace.common.jLibNoise.noise.module;

import ConquerSpace.common.jLibNoise.noise.NoiseGen;
import ConquerSpace.common.jLibNoise.noise.NoiseQuality;

/**
 *
 */
public class Perlin extends Module {

    // Default frequency
    public static final double DEFAULT_PERLIN_FREQUENCY = 1.0;
    // Default lacunarity
    public static final double DEFAULT_PERLIN_LACUNARITY = 2.0;
    // Default number of octaves
    public static final int DEFAULT_PERLIN_OCTAVE_COUNT = 6;
    // Default persistence value
    public static final double DEFAULT_PERLIN_PERSISTENCE = 0.5;
    // Default noise quality
    public static final NoiseQuality DEFAULT_PERLIN_QUALITY = NoiseQuality.QUALITY_STD;
    // Default noise seed
    public static final int DEFAULT_PERLIN_SEED = 0;

    // Maximum number of octaves for the noise::module::Perlin noise module.
    public static final int PERLIN_MAX_OCTAVE = 30;
    // Frequency of the first octave.
    protected double frequency;
    // Frequency multiplier between successive octaves.
    protected double lacunarity;
    // Quality of the Perlin noise.
    protected NoiseQuality noiseQuality;
    // Total number of octaves that generate the Perlin noise.
    protected int octaveCount;
    // Persistence of the Perlin noise.
    protected double persistence;
    // Seed value used by the Perlin-noise function.
    protected int seed;

    public Perlin() {
        super(0);

        frequency = DEFAULT_PERLIN_FREQUENCY;
        lacunarity = DEFAULT_PERLIN_LACUNARITY;
        noiseQuality = DEFAULT_PERLIN_QUALITY;
        octaveCount = DEFAULT_PERLIN_OCTAVE_COUNT;
        persistence = DEFAULT_PERLIN_PERSISTENCE;
        seed = DEFAULT_PERLIN_SEED;
    }

    /**
     * Returns the frequency of the first octave.
     *
     * @return The frequency of the first octave.
     */
    public double getFrequency() {
        return frequency;
    }

    /**
     * Returns the lacunarity of the Perlin noise.
     * <p>
     * The lacunarity is the frequency multiplier between successive octaves.
     *
     * @return The lacunarity of the Perlin noise.
     */
    public double getLacunarity() {
        return lacunarity;
    }

    /**
     * Returns the quality of the Perlin noise.
     *
     * @return The quality of the Perlin noise.
     * @see NoiseQuality for definitions of the various coherent-noise qualities.
     */
    public NoiseQuality getNoiseQuality() {
        return noiseQuality;
    }

    /**
     * Returns the number of octaves that generate the Perlin noise.
     * <p>
     * The number of octaves controls the amount of detail in the Perlin noise.
     *
     * @return The number of octaves that generate the Perlin noise.
     */
    public int getOctaveCount() {
        return octaveCount;
    }

    /**
     * Returns the persistence value of the Perlin noise.
     * <p>
     * The persistence value controls the roughness of the Perlin noise.
     *
     * @return The persistence value of the Perlin noise.
     */
    public double getPersistence() {
        return persistence;
    }

    /**
     * Returns the seed value used by the Perlin-noise function.
     *
     * @return The seed value.
     */
    public int getSeed() {
        return seed;
    }

    @Override
    public int getSourceModuleCount() {
        return 0;
    }

    @Override
    public double getValue(double x, double y, double z) {
        double value = 0.0;
        double signal;
        double curPersistence = 1.0;
        double nx;
        double ny;
        double nz;
        int localSeed;

        x *= frequency;
        y *= frequency;
        z *= frequency;

        for (int curOctave = 0; curOctave < octaveCount; curOctave++) {
            // Make sure that these floating-point values have the same range as a 32-
            // bit integer so that we can pass them to the coherent-noise functions.
            nx = NoiseGen.makeInt32Range(x);
            ny = NoiseGen.makeInt32Range(y);
            nz = NoiseGen.makeInt32Range(z);

            // Get the coherent-noise value from the input value and add it to the final result.
            localSeed = (this.seed + curOctave) & 0xffffffff;
            signal = NoiseGen.gradientCoherentNoise3D(nx, ny, nz, localSeed, noiseQuality);
            value += signal * curPersistence;

            // Prepare the next octave.
            x *= lacunarity;
            y *= lacunarity;
            z *= lacunarity;
            curPersistence *= persistence;
        }

        return value;
    }

    /**
     * Sets the frequency of the first octave.
     *
     * @param frequency The frequency of the first octave.
     */
    public void setFrequency(double frequency) {
        this.frequency = frequency;
    }

    /**
     * Sets the lacunarity of the Perlin noise.
     * <p>
     * The lacunarity is the frequency multiplier between successive octaves.
     * <p>
     * For best results, set the lacunarity to a number between 1.5 and 3.5
     *
     * @param lacunarity The lacunarity of the Perlin noise.
     */
    public void setLacunarity(double lacunarity) {
        this.lacunarity = lacunarity;
    }

    /**
     * Sets the quality of the Perlin noise.
     *
     * @param noiseQuality The quality of the Perlin noise.
     * @see NoiseQuality for definitions of the various coherent-noise qualities.
     */
    public void setNoiseQuality(NoiseQuality noiseQuality) {
        this.noiseQuality = noiseQuality;
    }

    /**
     * Sets the number of octaves that generate the Perlin noise.
     * <p>
     * The number of octaves controls the amount of detail in the Perlin noise.
     * <p>
     * The larger the number of octaves, the more time required to calculate the Perlin-noise value.
     *
     * @param octaveCount The number of octaves that generate the Perlin noise.
     * @throws IllegalArgumentException An invalid parameter was specified; see the preconditions
     * for more information. The number of octaves ranges from 1 to PERLIN_MAX_OCTAVE.
     */
    public void setOctaveCount(int octaveCount) {
        if (octaveCount < 1 || octaveCount > PERLIN_MAX_OCTAVE) {
            throw new IllegalArgumentException();
        }
        this.octaveCount = octaveCount;
    }

    /**
     * Sets the persistence value of the Perlin noise.
     * <p>
     * The persistence value controls the roughness of the Perlin noise.
     * <p>
     * For best results, set the persistence to a number between 0.0 and 1.0.
     *
     * @param persistence The persistence value of the Perlin noise.
     */
    public void setPersistence(double persistence) {
        this.persistence = persistence;
    }

    /**
     * Sets the seed value used by the Perlin-noise function.
     *
     * @param seed The seed value.
     */
    public void setSeed(int seed) {
        this.seed = seed;
    }
}
