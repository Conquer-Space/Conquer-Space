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
package ConquerSpace.common.game.city;

import ConquerSpace.common.ConquerSpaceGameObject;
import ConquerSpace.common.GameState;
import ConquerSpace.common.ObjectReference;
import ConquerSpace.common.StarDate;
import ConquerSpace.common.game.characters.Person;
import ConquerSpace.common.game.characters.PersonEnterable;
import ConquerSpace.common.game.city.area.Area;
import ConquerSpace.common.game.city.modifier.CityModifier;
import ConquerSpace.common.game.economy.GoodOrder;
import ConquerSpace.common.game.economy.Trader;
import ConquerSpace.common.game.logistics.SupplyNode;
import ConquerSpace.common.game.organizations.Administrable;
import ConquerSpace.common.game.population.Population;
import ConquerSpace.common.game.resources.ResourceStockpile;
import ConquerSpace.common.game.resources.ResourceTransfer;
import ConquerSpace.common.game.resources.StorageNeeds;
import ConquerSpace.common.game.resources.StoreableReference;
import ConquerSpace.common.game.universe.GeographicPoint;
import ConquerSpace.common.game.universe.UniversePath;
import ConquerSpace.common.game.universe.bodies.Planet;
import ConquerSpace.common.save.SaveStuff;
import ConquerSpace.common.save.Serialize;
import ConquerSpace.common.save.SerializeClassName;
import ConquerSpace.common.util.DoubleHashMap;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

/**
 *
 * @author EhWhoAmI
 */
@SerializeClassName("city")
public class City extends ConquerSpaceGameObject implements PersonEnterable,
        SupplyNode, Administrable, Trader {

    @Serialize("population")
    private ObjectReference population;

    public static final String CITY_DEFAULT = "emp";

    /**
     * The person who governs the place
     */
    private ObjectReference governor;

    @Serialize("name")
    private String name;

    @Serialize("areas")
    private ArrayList<ObjectReference> areas;

    @Serialize("working-for")
    private ArrayList<ObjectReference> workableFor;

    @Serialize("people")
    private ArrayList<ObjectReference> peopleAtCity;

    @Serialize(value = "resources", special = SaveStuff.Good)
    private HashMap<StoreableReference, Double> resources;

    @Serialize(value = "demands", special = SaveStuff.Good)
    public DoubleHashMap<StoreableReference> resourceDemands;

    private HashMap<ResourceStockpile, DoubleHashMap<StoreableReference>> resourceExports;
    private HashMap<ResourceStockpile, DoubleHashMap<StoreableReference>> resourceImports;

    @Serialize("storage-needs")
    private ArrayList<StorageNeeds> storageNeeds;
    //public ArrayList<PopulationUnit> population;

    private ArrayList<ObjectReference> supplySegments;

    private GeographicPoint initialPoint = null;

    @Serialize("max-storage")
    private int maxStorage;

    private ArrayList<ObjectReference> supplyChains;

    private int ledgerClearDelta = 0;
    private HashMap<StoreableReference, DoubleHashMap<String>> resourceLedger;

    private HashSet<StoreableReference> primaryProduction;
    private DoubleHashMap<StoreableReference> previousQuarterProduction;

    public ObjectReference market;

    @Serialize("location")
    private ObjectReference location;

    @Serialize("tags")
    private HashMap<String, Integer> tags;

    //% to completing a unit
    @Serialize("population-completion")
    private float populationUnitPercentage = 0;

    //Growth rates of the species...
    //private HashMap<Race, Float> speciesRates;
    private boolean resetJobs = false;

    @Serialize("city-type")
    private CityType cityType;

    //Size in tiles
    @Serialize("tiles")
    private int size;

    private ArrayList<CityModifier> cityModifiers;

    //Energy needed
    private int energyProvided;

    //Energy needed
    private int energyNeeded;

    private ObjectReference ownerReference;

    private int wealth = 0;

    public City(GameState gameState, ObjectReference location) {
        super(gameState);
        workableFor = new ArrayList<>();
        ownerReference = null;
        areas = new ArrayList<>();
        storageNeeds = new ArrayList<>();
        resources = new HashMap<>();
        resourceExports = new HashMap<>();
        resourceImports = new HashMap<>();
        //jobProcessor = new JobProcessor();
        this.location = location;
        peopleAtCity = new ArrayList<>();

        supplySegments = new ArrayList<>();

        Population population = new Population(gameState);
        this.population = population.getReference();

        resourceLedger = new HashMap<>();
        resourceDemands = new DoubleHashMap<>();
        tags = new HashMap<>();
        cityModifiers = new ArrayList<>();
        primaryProduction = new HashSet<>();

        previousQuarterProduction = new DoubleHashMap<>();

        cityType = CityType.Generic;
        size = 0;

        energyProvided = 0;
    }

    public void setPopulationUnitPercentage(float populationUnitPercentage) {
        this.populationUnitPercentage = populationUnitPercentage;
    }

    public float getPopulationUnitPercentage() {
        return populationUnitPercentage;
    }

    @Override
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

    @Override
    public UniversePath getUniversePath() {
        return gameState.getObject(location, Planet.class).getUniversePath();
    }

    public Person getGovernor() {
        return gameState.getObject(governor, Person.class);
    }

    public void setGovernor(Person governor) {
        governor.setRole("Governing " + name);
        this.governor = governor.getReference();
    }

    public boolean toResetJobs() {
        return resetJobs;
    }

    public void resetJobs() {
        resetJobs = true;
    }

    public void doneResettingJobs() {
        resetJobs = false;
    }

    @Override
    @SuppressWarnings("unchecked")
    public ArrayList<Person> getPeopleArrayList() {
        return new ArrayList();
    }

    public void incrementPopulation(StarDate date, long delta) {
        //Handled somewhere else, if i'm not wrong
    }

    @Override
    public void addResourceTypeStore(StoreableReference type) {
        resources.put(type, 0d);
    }

    @Override
    public Double getResourceAmount(StoreableReference type) {
        return resources.get(type);
    }

    @Override
    public void addResource(StoreableReference type, Double amount) {
        if (!resources.containsKey(type)) {
            resources.put(type, 0d);
        }

        resources.put(type, resources.get(type) + amount);
    }

    @Override
    public boolean canStore(StoreableReference type) {
        return true;//(resources.containsKey(type));
    }

    @Override
    public StoreableReference[] storedTypes() {
        Iterator<StoreableReference> res = getResources().keySet().iterator();
        StoreableReference[] arr = new StoreableReference[getResources().size()];
        int i = 0;
        while (res.hasNext()) {
            StoreableReference next = res.next();
            arr[i] = next;
            i++;
        }
        return arr;
    }

    @Override
    public boolean removeResource(StoreableReference type, Double amount) {
        //Get the amount in the place
        if (!resources.containsKey(type)) {
            return false;
        }

        Double currentlyStored = getResources().get(type);
        if (amount > currentlyStored) {
            return false;
        }
        return true;
    }

    @Override
    public boolean hasResource(StoreableReference type) {
        return resources.containsKey(type);
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public void incrementSize() {
        size++;
    }

    public void addArea(ObjectReference a) {
        getAreas().add(a);
    }

    public CityType getCityType() {
        return cityType;
    }

    public void setCityType(CityType cityType) {
        this.cityType = cityType;
    }

    public void clearLedger(int delta) {
        ledgerClearDelta = delta;
        getResourceLedger().clear();
    }

    public int getLedgerClearDelta() {
        return ledgerClearDelta;
    }

    public int getEnergyProvided() {
        return energyProvided;
    }

    public void setEnergyProvided(int energyProvided) {
        this.energyProvided = energyProvided;
    }

    public void incrementEnergy(int amount) {
        this.energyProvided += amount;
    }

    public int getEnergyNeeded() {
        return energyNeeded;
    }

    public void setEnergyNeeded(int energyNeeded) {
        this.energyNeeded = energyNeeded;
    }

    public double getUnemploymentRate() {
        long currentlyWorking = 0;
        for (ObjectReference areaId : getAreas()) {
            Area area = gameState.getObject(areaId, Area.class);
            currentlyWorking += area.getCurrentlyManningJobs();
        }
        //return (population)
        long populationSize = gameState.getObject(getPopulation(), Population.class).getWorkableSize();
        return ((double) (populationSize - currentlyWorking) / (double) populationSize);
    }

    public ObjectReference getOwner() {
        return ownerReference;
    }

    public void setOwner(ObjectReference owner) {
        this.ownerReference = owner;
    }

    @Override
    public ArrayList<ObjectReference> getSupplyConnections() {
        return supplySegments;
    }

    @Override
    public void addSupplyConnection(ObjectReference reference) {
        supplySegments.add(reference);
    }

    public GeographicPoint getInitialPoint() {
        return initialPoint;
    }

    public void setInitialPoint(GeographicPoint initialPoint) {
        this.initialPoint = initialPoint;
    }

    public ObjectReference getLocation() {
        return location;
    }

    @Override
    public int getWealth() {
        return wealth;
    }

    @Override
    public ArrayList<GoodOrder> getRequests() {
        //IDK process?
        return new ArrayList<>();
    }

    @Override
    public ArrayList<GoodOrder> getSellOrders() {
        return new ArrayList<>();
    }

    public ObjectReference getMarket() {
        return market;
    }

    public void setMarket(ObjectReference market) {
        this.market = market;
    }

    @Override
    public void changeWealth(int amount) {
        wealth += amount;
    }

    public synchronized HashMap<ResourceStockpile, DoubleHashMap<StoreableReference>> getResourceImports() {
        return resourceImports;
    }

    public synchronized HashMap<ResourceStockpile, DoubleHashMap<StoreableReference>> getResourceExports() {
        return resourceExports;
    }

    public synchronized DoubleHashMap<StoreableReference> getPreviousQuarterProduction() {
        return previousQuarterProduction;
    }

    /**
     * @return the population
     */
    public ObjectReference getPopulation() {
        return population;
    }

    /**
     * @return the areas
     */
    public ArrayList<ObjectReference> getAreas() {
        return areas;
    }

    /**
     * @return the workableFor
     */
    public ArrayList<ObjectReference> getWorkableFor() {
        return workableFor;
    }

    /**
     * @return the peopleAtCity
     */
    public ArrayList<ObjectReference> getPeopleAtCity() {
        return peopleAtCity;
    }

    /**
     * @return the resources
     */
    public HashMap<StoreableReference, Double> getResources() {
        return resources;
    }

    /**
     * @return the storageNeeds
     */
    public ArrayList<StorageNeeds> getStorageNeeds() {
        return storageNeeds;
    }

    /**
     * @return the supplyChains
     */
    public ArrayList<ObjectReference> getSupplyChains() {
        return supplyChains;
    }

    /**
     * @return the resourceLedger
     */
    public HashMap<StoreableReference, DoubleHashMap<String>> getResourceLedger() {
        return resourceLedger;
    }

    /**
     * @return the primaryProduction
     */
    public HashSet<StoreableReference> getPrimaryProduction() {
        return primaryProduction;
    }

    /**
     * @return the tags
     */
    public HashMap<String, Integer> getTags() {
        return tags;
    }

    /**
     * @return the cityModifiers
     */
    public ArrayList<CityModifier> getCityModifiers() {
        return cityModifiers;
    }

    public void clearLedgers() {
        getResourceLedger().clear();
        getResourceExports().clear();
        getResourceImports().clear();
        getPreviousQuarterProduction().clear();
        getPrimaryProduction().clear();
    }

    @Override
    public void preResourceTransfer(StoreableReference type, Double amount, ResourceStockpile toWhere) {
        //Do nothing
    }

    @Override
    public void postResourceTransfer(StoreableReference resource, Double amount, ResourceStockpile toWhere) {
        String transferType;
        if (amount > 0) {
            transferType = "added";
        } else {
            transferType = "removed";
        }

        //Add to ledger
        DoubleHashMap<String> ledger;
        if (resourceLedger.containsKey(resource)) {
            ledger = resourceLedger.get(resource);
            ledger.addValue(transferType, (amount));
        } else {
            ledger = new DoubleHashMap<>();
            ledger.put(transferType, amount);
        }
        
        resourceLedger.put(resource, ledger);

        if (toWhere instanceof ConquerSpaceGameObject && areas.contains(((ConquerSpaceGameObject) toWhere).getReference())) {
            return;
        }

        //Add imports and exports
        DoubleHashMap<StoreableReference> map;
        if (amount > 0) {
            //Importing
            if (resourceImports.containsKey(toWhere)) {
                map = resourceImports.get(toWhere);
            } else {
                map = new DoubleHashMap<>();
            }

            //Do ledger
            map.addValue(resource, amount);
            resourceImports.put(toWhere, map);
        } else {
            //Exporting
            if (resourceExports.containsKey(toWhere)) {
                map = resourceExports.get(toWhere);
            } else {
                map = new DoubleHashMap<>();
            }

            //Do ledger
            map.addValue(resource, amount);
            resourceExports.put(toWhere, map);
        }
    }
}
