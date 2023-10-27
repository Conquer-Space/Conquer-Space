Eonomy
---
The economy is consisted of two different parts: intracity trade and intercity trade.

### Terminology
| Term    | Description |
| -------- | ------- |
| Agent | An entity that trades on the market, such as a factory. |

## Individual agents

Agents have a resource input, which can be multiple inputs, but only one output. They will have a multiplier, called the utilization, that multiplies the inputs and outputs. This represents the factory increasing and reducing their production so that they don't lose money.


But, along with production costs, the factory also has a base cost, the capital cost, for maintenance based off the size of the factory. The factory will always use this amount of resources.


Agents have a working force which is homogenous. They will pay the workers an even wage, and this is deposited into the population wallet for them to spend.


The agent will try to balance their inputs and outputs so that they stay profitable. They can stay profitable by decreasing the price, increasing supply

General code locations:
 - [common/components/resource.h](https://github.com/Conquer-Space/Conquer-Space/blob/main/src/common/components/resource.h#L237) For the components of the 
  - [cqsp::common::systems::SysProduction](https://github.com/Conquer-Space/Conquer-Space/blob/main/src/common/systems/economy/sysfactory.cpp) For the implementation in the resource generation of the factories

### Notes:
I think we should add multiple resource outputs, and it should not be too hard to implement it.
The main consideration is that we need to account for byproducts. How would the factories determine if it's generating too much of the byproduct? Do they need to reduce production?


We should also have factories hire from different strata and population segments, so that we can have discrimination and social and economic stratification.


We should also account for the minimum price a factory will sell their outputs for. If the factory makes a zero or even negative return on each good, it will go bankrupt. The factory should try to stay afloat.


The main reason for this is that when there is a sudden collapse in demand, then the price of the good also collapse, and all the producers will go bankrupt.
However, since the price of goods is not fixed, when the market recovers, that is, the supply demand ratio recovers, the price can be fixed at the lower price, and more factories will go bankrupt than intended. Subsequently, the price will spike up and down, oscilliating until the market stabilizes. However, we don't want the oscilitation, and want the prices to remain stable and smooth.

## Intracity Trade
Intracity trade is the lowest level a city will trade at. Agents will trade within the city first, before trading in across cities.

### Price determination
Price is determined by taking the sum of the inputs (demand) and the sum of the outputs(supply), and getting the ration between the both. This is the supply demand ratio. 

Then, for each good, the market will increase or reduce the price of the good. If the supply demand ratio is greater than 1, supply is higher than demand, so the price will decrease. If the supply demand ratio is below 1, then the demand is higher than 1, so the price will increase.

There are a few extremes. If supply is zero and demand is greater than zero, then there is no supply. According to the algorithm, the price will go up to infinity.
Similarly, if the demand is zero and supply is greater than zero, the price will collapse to zero.

For now, the price will continue to go in the directions that they already do, but in the future, we should create some sort of bound so that price doesn't change.

Next time, price determination should be based off the SD ratio so that an extremely high supply and demand ratio will balance itself quickly.

## Intercity Trade
In intercity trade, the factories will trade between each city. The global economy will form a connected graph. These represent the most efficient shipping method in terms of cost. For land based routes, it will be trucks or trains, depending on the infrastructure, and ships for sea based routes. For extreme cases, such as antartica, it could be aircraft.
 This will be a wacky thing and I hope it's not too bad
Infrastructure and transport costs, along with taxes and tariffs
 - will not implement tariffs on goods coming from own country
Price determining
 - Not sure how
Who to trade with
 - Own country first then other countries
When to trade with outside

Assumptions:
 - Agents will tend to stick with who they are already purchasing goods with unless the difference in price is high.
 - Agents will try to minimize costs and maximize profits
 - Agents will try to break even, which means that they will stop selling goods after the cost to produce a good is higher than the selling price.
 - Agents will try to do stuff