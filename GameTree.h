#ifndef GAMETREE_H
#define GAMETREE_H

/*
   When constructing the game tree, we need to take into account
   that we have a graph, where a particular game state can be
   reach from different paths, so we need to a mechinism when
   building the graph to join nodes.
   We also won't be able to compute the full game tree, so
   we need to have a way to declare what we want the conditions
   to be. Off the top of my head, I guess that because of the
   complexity involved, we will want to compute the game tree
   for a depth n, then prune the game tree for some metric, and
   then do the iteration m times, or this would be dynamic.
   Another case to consider is that we should be able to
   start with an origin of two kings, and then get a game tree
   with no one winning. Similarly, when we start with two kings
   and one rook, we will get an game tree with a winner.
   The game tree code should not be trivial, we should have
   a mechinism for multi-threaded, user directed game tree
   building.
        I think multi-threaded pruning can work by marking 
   a parent.



   */

namespace CC{
namespace GT{

        struct Node;

        struct NodeVisitor{
        };


        struct Node{
                std::vector<GameTreeNode*> parents_;
                std::vector<GameTreeNode*> children_;
                Move* last_move_{nullptr}; // the move that got us to here
        };

        struct NodeFactory{
                virtual Node* Make(){ return new node; }
        };
        

        /*
                Here is an idea, 
         */

        struct NodeFactoryDevice{
                Node* Get(ChessInstance* instance ){
                        static ChessInstanceHasher hasher;
                        auto hash = hasher(instance);
                        auto iter = world_.find(hash);
                        if( iter == world_.end() ){
                                return iter->second;
                        }
                        auto ptr = this->Make(instance);
                        for( auto obs : on_create_ ){
                                obs(ptr);
                        }
                        return ptr;
                }
                void AddNotify( std::function<void(ChessInstance* instance, node*)> f){
                        on_create_.push_back(std::move(f));
                }
        private:
                std::map<std::string, Node*> world_;
                std::vector< std::function<void(Node*)> > on_create_;
        };

        struct PathContext{
        };

        // this directrs the game tree. For example we could have two personalities
        // for the game tree, one super aggreive one, which always looks to take the
        // path which captures/exchanges peices, and another one which looks to 
        // control the center four peices.
        //        This will also allow combining of Personalities, for example 
        // opening positions will be a personality, and we will be able to combine
        // these with an aggrasive personality.
        struct Personality;

        // this takes are of how the tree is built. For example we could use
        // the same personality, with a different marshals, one creates a super
        // big game tree, whilst the other builds as much as it can in one minute
        struct Marshal;


        /* 
                The personality is the mechinism where we can direct the 
                game tree to follow standard chess openings (as to gie the
                AI a personality ).
        */
        struct Personality{
                /*
                        There will probably be trivial cases which make the computation
                        complex which can easily be handled by simple conditions 
                 */
                virtual bool ShortCircuitMove(ChessInstance* ){ return false; }
                /*
                        This gives the marshal an option to cull the 
                        the possibke moves
                */
                virtual boost::optional<std::vector<Move*> > PossibleMoves(ChessInstance* instance){
                        return boost::none;
                }

        };
        struct DefaultPersonality : Personality{
        };


        struct Controller{
                void Prune(Node*);
                void 
                
                void MarkDead(Node*);
        };

        struct Marshal{
                virtual bool 
        };


        /*
                Do we want to schedule every node on a level first,
                or do we want to go to the maximum depth down a path
                first
         */
        struct Schedualer{
                virtual ~Schedualer()=default;
                virtual void Add(Node* node)=0;
                virtual Node* Pop()=0;
        };

        struct Worker{
                /*
                        Precondition node is still a running game
                */
                void Generate(Node* node, ChessInstance* inst){


                        std::vector<Move*> moves;
                        prsly_->CandidateMoves(inst, moves);

                        for(auto m : moves){
                                ChessInstance* next = new ChessInstance{*inst};
                                m->Apply( next );
                                if( prsly_->ShortCircuitMove( node, m, inst, next ) ){
                                        continue;
                                }
                                auto child = device_->Get(next);
                                node->Adopt(child);

                        }

                }
        private:
                NodeFactoryDevice* device_;
                Marshal* mrsh_;
                Schedualer* sched_;
                Personality* prsly_;
                Controller* ctrl_;
        };

        struct Schedualer{
                Schedualer(){

                        device_->AddNotify( [this](Node* node){
                                this->OnNewNode_(node);
                        });
                }
                void Start(Node* node){
                }
        private:
                void OnNewNode_(Node* node){
                        if( !  mrsh_->ShouldDecend(node) ){
                                ctrl_->MarkDead(node);
                                continue;
                        }
                }
                NodeFactoryDevice* device_;
                Marshal* mrsh_;
                Schedualer* sched_;
                Personality* prsly_;
                Controller* ctrl_;
        };

} // end namespace GT

struct GameTreeBuilderContext{
};

struct GameTreeBuilder{
        GameTreeNode* Build( ChessInstance* origin );
};
struct GameTreeVisitor{

};

}  // end namespace CC

#endif // GAMETREE_H
