#ifndef UCI_H_
#define UCI_H_

#include "utils.h"

namespace pismo
{

namespace UCI
{
	/* Initializes UCI by sending 
	   initial information to GUI
	   through stdout
	 */
	void initUCI();

	/* Manages UCI by listening to
	   stdin and executing the commands
	   coming from GUI
	 */
	void manageUCI();

	/* Manages the searches of the engine
	   by starting the engine and 
	   printing the move
	*/
	void manageSearch();

	/* Manages the timer that changes
	   the condition variable once time
	   is over
	   */
	void manageTimer();

	/* Prints move to stdout according to 
	   UCI format
	*/
	void printMove(const MoveInfo& move);

}

}

#endif //UCI_H_
