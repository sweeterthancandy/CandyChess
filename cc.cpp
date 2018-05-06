//
//  main.cpp
//  CC
//
//  Created by dest royer on 28/04/2018.
//  Copyright © 2018 dest royer. All rights reserved.
//

#include <iostream>
#include <array>
#include <vector>
#include <cstdint>

/*


 */

namespace CC{
        /*

           chess
           game logic
           game tree
           complexity management


           solve end game situations
           compute "best" moves
           */


        /*
           4 bits for type, one bit for black/white, = 5 bits per square
           64 sqares per board. We store the types of peices in a
           4 * 64bit vector, and the types of the squares in a 64bit
           each.
           For the game meta-data, we need to store whos turn it is,
           and if the king can castle king-side of queen-side. This
           can be encoded in no less than three bits.
           */

        struct ChessInstance;




        struct BoardDeltaOperator{
                enum OperatorKind{
                        OP_Move,
                        OP_LeftCastle,
                        OP_RightCastle,
                };
        };







        namespace Detail{
                struct ChessInstanceHasher{
                        std::string operator()(ChessInstance* inst)const;
                };
        } // end namespace Detail


        struct ChessGameLogic{
                // 
                boost::optional<std::string> ApplyMove(ChessInstance* instance, Move* move);
        };



} // end namespace CC

int main(int argc, const char * argv[]) {
        // insert code here...
        std::cout << "Hello, World!\n";
        return 0;
}
