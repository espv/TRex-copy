/*
 * Copyright (C) 2011 Francesco Feltrinelli <first_name DOT last_name AT gmail DOT com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RuleR1.hpp"
#include "RuleR0.hpp"
#include "../../../TRex2-lib/src/Packets/RulePkt.h"
#include "../../../../../../../../../../usr/include/c++/5/cstring"

using namespace concept::test;

RulePkt* RuleR1::buildRule(int event_temp, int event_humidity, int event_fire, int temperature) {
	RulePkt* rule= new RulePkt(false);

	// predicate 0 is the root predicate
    int indexRootPredicate= 0;
    //int indexPredTemp= 1;
	int indexSecondPredicate= 1;
	//int indexPredSmoke= 0;

	TimeMs fiveMin(1000*60*5);

	// Smoke root predicate
	// Fake constraint as a temporary workaround to an engine's bug
	// FIXME remove workaround when bug fixed

	// Temp predicate
	// Constraint: Temp.value > 45
	Constraint tempConstr[1];
	strcpy(tempConstr[0].name, RuleR0::ATTR_TEMPVALUE);
	tempConstr[0].type= INT;
	tempConstr[0].op= EQ;
	tempConstr[0].intVal= temperature;

    Constraint humidityConstr[1];
    strcpy(humidityConstr[0].name, "percentage");
    humidityConstr[0].type= INT;
    humidityConstr[0].op = LT;
    humidityConstr[0].intVal = 25;
    //rule->addRootPredicate(event_humidity, humidityConstr, 1);
    rule->addRootPredicate(event_temp, tempConstr, 1);//, indexSecondPredicate, fiveMin, LAST_WITHIN);
    rule->addPredicate(event_humidity, humidityConstr, 1, indexRootPredicate, fiveMin, LAST_WITHIN);
    rule->addConsuming(event_humidity);
    rule->addConsuming(event_temp);
    //rule->addPredicate(event_temp, tempConstr, 0, indexRootPredicate, fiveMin, LAST_WITHIN);

	// Fire template
	CompositeEventTemplate* fireTemplate= new CompositeEventTemplate(event_fire);
	// Area attribute in template
	OpTree* areaOpTree= new OpTree(new RulePktValueReference(indexRootPredicate, RuleR0::ATTR_AREA, STATE), STRING);
	fireTemplate->addAttribute(RuleR0::ATTR_AREA, areaOpTree);
	// MeasuredTemp attribute in template
	OpTree* measuredTempOpTree= new OpTree(new RulePktValueReference(indexRootPredicate, RuleR0::ATTR_TEMPVALUE, STATE), INT);

	/*ComplexParameter par;
	par.operation = EQ;
	par.type = STATE;
	par.leftTree = new OpTree(new RulePktValueReference(indexRootPredicate, RuleR0::ATTR_AREA, STATE), STRING);
	par.rightTree = new OpTree(new RulePktValueReference(indexSecondPredicate, RuleR0::ATTR_AREA, STATE), STRING);
	par.vtype = STRING;
	rule->addComplexParameter(par.operation, par.vtype, par.leftTree,
                              par.rightTree);*/

    // Parameter: Smoke.area=Temp.area
    //rule->addParameterBetweenStates(indexRootPredicate, RuleR0::ATTR_AREA, indexSecondPredicate, RuleR0::ATTR_AREA);

	fireTemplate->addAttribute(RuleR0::ATTR_MEASUREDTEMP, measuredTempOpTree);

	rule->setCompositeEventTemplate(fireTemplate);

	return rule;
}

SubPkt* RuleR1::buildSubscription(int event_fire) {
	Constraint constr[1];
	// Area constraint
	strcpy(constr[0].name, RuleR0::ATTR_AREA);
	constr[0].type= STRING;
	constr[0].op= EQ;
	strcpy(constr[0].stringVal, RuleR0::AREA_OFFICE);

	return new SubPkt(event_fire, constr, 1);
}

vector<PubPkt*> RuleR1::buildPublication(int event_temp, int event_humidity, int temperature) {
    // Temp event
    Attribute tempAttr[2];
    // Value attribute
    strcpy(tempAttr[0].name, RuleR0::ATTR_TEMPVALUE);
    tempAttr[0].type= INT;
    tempAttr[0].intVal= temperature;
    // Area attribute
    strcpy(tempAttr[1].name, RuleR0::ATTR_AREA);
    tempAttr[1].type= STRING;
    strcpy(tempAttr[1].stringVal, RuleR0::AREA_OFFICE);
    PubPkt* tempPubPkt= new PubPkt(event_temp, tempAttr, 2);

    // Smoke event
    // Area attribute
    Attribute humidityAttr[2];
    strcpy(humidityAttr[0].name, "percentage");
    humidityAttr[0].type= INT;
    humidityAttr[0].intVal= 22;

    strcpy(humidityAttr[1].name, RuleR0::ATTR_AREA);
    humidityAttr[1].type= STRING;
    strcpy(humidityAttr[1].stringVal, RuleR0::AREA_OFFICE);
    PubPkt* humidityPubPkt= new PubPkt(event_humidity, humidityAttr, 2);

    vector<PubPkt*> pubPkts;
    pubPkts.push_back(tempPubPkt);
    pubPkts.push_back(humidityPubPkt);
    return pubPkts;
}
