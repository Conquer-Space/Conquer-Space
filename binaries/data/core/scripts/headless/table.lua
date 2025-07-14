-- Prints a market table
function print_market_table(market_table)
    print(string.rep("=", 110))
    print("Good\t\t\t| Price   \t| S/D Ratio | Supply | Demand | Consumption | Production | Trade")
    print(string.rep("=", 110))
    for key, value in pairs(market_table) do
        table_line = string.format("%-20s \t| %13.e | %9.e | %6.e | %6.e | %11.e | %10.e | %10.e",
                                            core.get_name(key), 
                                            value["price"],
                                            value["sd_ratio"],
                                            value["supply"],
                                            value["demand"],
                                            value["consumption"],
                                            value["production"],
                                            value["trade"]
                                    )
        print(table_line)
    end
end

-- Prints the table of a city's market
function print_city_market(city_name)
    city = core.get_city(city_name)
    table = core.get_market_table(city)
    print_market_table(table)
end
