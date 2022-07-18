
import threading, time
from citrine_module_nl import citrine_eval_nl
from citrine_module_en import citrine_eval_en

def callback(bericht, *args):
	return str(sum(map(int,list(args))))

print( 
citrine_eval_en(
"""
	✎ write: ((Python add: 1 and: 2 and: 3) object), stop.
""",
callback)
)

print( 
citrine_eval_nl(
"""
	✎ schrĳf: ((Python optellen: 4 en: 5 en: 6) object), stop.
""",
callback)
)

