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

#include "RuleR0.hpp"
#include "../../../TRex2-lib/src/Packets/RulePkt.h"
#include "../../../TRex2-lib/src/Packets/PubPkt.h"
#include "../../../TRex2-lib/src/Packets/SubPkt.h"

using namespace concept::test;

char RuleR0::ATTR_TEMPVALUE[]= "value";
char RuleR0::ATTR_AREA[]= "area";
char RuleR0::ATTR_MEASUREDTEMP[]= "measuredTemp";

char RuleR0::AREA_GARDEN[]= "garden";
char RuleR0::AREA_OFFICE[]= "office";
char RuleR0::AREA_TOILET[]= "toilet";

RulePkt* RuleR0::buildRule(){
  auto rule= new RulePkt(false);

	int indexPredTemp= 0;

	// Temp root predicate
	Constraint tempConstr[3];
	strcpy(tempConstr[0].name, ATTR_TEMPVALUE);
	tempConstr[0].type= INT;
	tempConstr[0].op= GT;
	tempConstr[0].intVal= 45;
	strcpy(tempConstr[1].name, "integer_constraint_2");
	tempConstr[1].type= INT;
	tempConstr[1].op= LT;
	tempConstr[1].intVal= 1000;
	strcpy(tempConstr[2].name, "area");
	tempConstr[2].type = STRING;
	tempConstr[2].op = EQ;
	strcpy(tempConstr[2].stringVal, "office");
	rule->addRootPredicate(EVENT_TEMP, tempConstr, 3);

	// Fire template
	auto fireTemplate= new CompositeEventTemplate(EVENT_FIRE);

	// Area attribute in template
  auto areaOpTree= new OpTree(new RulePktValueReference(indexPredTemp, ATTR_AREA, STATE), STRING);
	fireTemplate->addAttribute(ATTR_AREA, areaOpTree);
	// MeasuredTemp attribute in template
  auto measuredTempOpTree= new OpTree(new RulePktValueReference(indexPredTemp, ATTR_TEMPVALUE, STATE), INT);
	fireTemplate->addAttribute(ATTR_MEASUREDTEMP, measuredTempOpTree);

	rule->setCompositeEventTemplate(fireTemplate);

	return rule;
}

SubPkt* RuleR0::buildSubscription() {
	Constraint constr[1];
	// Area constraint
	strcpy(constr[0].name, ATTR_AREA);
	constr[0].type= STRING;
	constr[0].op= EQ;
	strcpy(constr[0].stringVal, AREA_OFFICE);

	return new SubPkt(EVENT_FIRE, constr, 1);
}

std::vector<PubPkt*> RuleR0::buildPublication(){
	Attribute attr[3];
	// Value attribute
	strcpy(attr[0].name, ATTR_TEMPVALUE);
	attr[0].type= INT;
	attr[0].intVal= 50;

	strcpy(attr[1].name, "integer_constraint_2");
	attr[1].type = INT;
	attr[1].intVal=88;
	// Area attribute
	strcpy(attr[2].name, ATTR_AREA);
	attr[2].type= STRING;
	strcpy(attr[2].stringVal, AREA_OFFICE);
	auto pubPkt= new PubPkt(EVENT_TEMP, attr, 3);

	std::vector<PubPkt*> pubPkts;
	pubPkts.push_back(pubPkt);
	return pubPkts;
}
