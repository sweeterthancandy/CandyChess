#ifndef CHESSINSTANCE_H
#define CHESSINSTANCE_H

#include <iostream>
#include <array>
#include <vector>
#include <cstdint>



namespace CC{
        
        using uchar = unsigned char;
        // signed
        using cord_t = char;
        
        struct tag_relative{};
        struct tag_absolute{};


        template<class Tag>
        struct BasicCoordinate{
                Relative(cord_t x, cord_t y):X{x},Y{y}{}

                uchar LinearProjection()const{
                        if( 0 <= X && X <= 7 ){
                                if( 0 <= Y && Y <= 7 ){
                                        return x_  + y_ * 8;
                                }
                        }
                        return LPOutOfBounds;
                }
                uchar X;
                uchar Y;
        };
        using Relative = BasicCoordinate<tag_relative>;
        using Absolute = BasicCoordinate<tag_absolute>;
        
        enum GameResult{
                GR_Running,
                GR_Draw,
                GR_StaleMate,
                GR_WhiteWin,
                GR_BlackWin,
                GR_SoftStop,
        };

        enum PieceKind{
                P_NotAPiece = 0,
                P_Pawn = 1,
                P_Rook = 2,
                P_Knight = 3,
                P_Bishop = 4,
                P_Queen = 5,
                P_King = 6,
        };
        
        struct Piece{

                static Piece* GetPawn(Side side);
                static Piece* GetKing(Side side);
                static Piece* GetQueen(Side side);

                virtual ~Piece()=default;

                virtual char Glyph()const=0;
                virtual void PossibleMoves(ChessInstance* board, uchar x, uchar y, std::vector<Move*>& result )const=0;
        };


        
        struct Move{
                static Move* GetMoveTo();
                static Move* GetCastle();
                static Move* GetPromote();
                static Move* GetCapturePromote();
                // im not going to implement this, outside scope of project
                static Move* GetImpasse();

                virtual void Apply( ChessInstance* instance )const=0;
        };

        struct MoveTo : Move{
                uchar from_x_;
                uchar from_y_;
                uchar to_x_;
                uchar to_y_;
        };

        struct ChessState{
                bool white_kingside_option{true};
                bool white_queenside_option{true};
                bool black_kingside_option{true};
                bool black_queenside_option{true};
        };

        enum SideType{
                Side_NotASide,
                Side_White,
                Side_Black
        };

        enum ClassificationType{
                CT_OutOfBounds,
                CT_Empty,
                CT_Opponent,
                CT_Friend,
        };

        static uchar LPOutOfBounds = 64;
        
        /*
           Chess Instance is a game representation appropriate
           for computing the game tree. We store object to represent
           the peices, because the game tree can be expressed as
           a series of moves from the origin, so each node can just
           have pointer to the parent and the move.
           */
        struct ChessInstance{

                auto Classify(SideType side, Absolute const& abs){
                        auto offset = abs.LinearProjection();
                        if( offset == LPOutOfBounds ){
                                return CT_OutOfBounds;
                        }
                        if( tiles_[offset] == nullptr ){
                                return CT_Empty;
                        }
                        return ( tiles_[offset]->Side() == side ?  CT_Friend : CT_Opponent );
                }

                void Set(Absolute const& abs, Piece* ptr){
                        tiles_[ abs.LinearProjection() ] = ptr;
                }
                void Next(ChessInstance* ptr){ next_.push_back(ptr); }

                std::array< Piece* , 64 > tiles_;
                uchar depth_{0};
                Side turn_{Side_White};
                GameResult result_{GR_Running}; 

                std::vector<ChessInstance*> next_;
        };


        struct Transform{
                // in this case, the base is not used
                virtual Absolute ToAbsolute(Absolute const& base, Absolute const& abs)const=0;
                virtual Absolute ToAbsolute(Absolute const& base, Relative const& rel)const=0;

                static Transform* GetForSide(Side side);
        };
        struct TransformIdentity : Transform{
                virtual Absolute ToAbsolute(Absolute const& base, Absolute const& abs)const{
                        return abs;
                }
                virtual Absolute ToAbsolute(Absolute const& base, Relative const& rel)const{
                        return Absolute{ base.X() + rel.X(), base.Y() + rel.Y() };
                }
        };
        /*
                For when it's black
         */
        struct Transform180 : Transform{
                virtual Absolute ToAbsolute(Absolute const& base, Absolute const& abs)const{
                        return Absolute{ 7 - abs.X(), 7 - abs.Y() };
                }
                virtual Absolute ToAbsolute(Absolute const& base, Relative const& rel)const{
                        return Absolute{ base.X() + ( 7 - rel.X() ), base.Y() + ( 7 - rel.Y() ) };
                }
        };
                
        Transform* Transform::GetForSide(Side side){
                if( side == Side_Black )
                        return &black;
                return &white;
        }


        struct ChessPieceView{

                ChessPieceView(ChessInstance* instance,
                               Transform* tranform,
                               std::vector<Move*>* moves,
                               Absolute pos):
                        instance_{instance},
                        tranform_{tranform},
                        moves_{moves},
                        pos_{pos}
                {}

                template<class CordType>
                auto Classify( Cord const& cord ){
                        auto abs = tranform_->ToAbsolute(pos_, cord);
                        return instance_->Classify(instance_->Side(), abs);
                }


                template<class CordType>
                void EmitCapture( Cord const& cord ){
                        auto abs = tranform_->ToAbsolute(pos_, cord);
                        moves_->push_back( Move::GetCapture( abs ) );
                }
                template<class CordType>
                void EmitMoveTo( Cord const& cord ){
                        auto abs = tranform_->ToAbsolute(pos_, cord);
                        moves_->push_back( Move::GetMoveTo( abs ) );
                }
                template<class CordType>
                void EmitPromote( Cord const& cord ){
                        auto abs = tranform_->ToAbsolute(pos_, cord);
                        moves_->push_back( Move::GetPromote( abs ) );
                }
        private:
                ChessInstance* instance_;
                Transform* tranform_;
                std::vector<Move*>* moves_;
                Absolute pos_;
        };
        
        struct MoveRuleBook{
                // returns false if rel is out of bounds or friend
                bool EmitConditionMoveOrCapture( ChessPieceView* view, Relative const& rel)const{
                        switch(view->Classify( rel ) ){
                        case CT_OutOfBounds:
                        case CT_Friend:
                                return false;
                        case CT_Empty:
                                view.EmitMoveTo( rel );
                                return true;
                        case CT_OutOfBounds:
                                view.EmitCapture( rel );
                                return true;
                        }
                        assert(false);
                }
                void EmitForEach( ChessPieceView* view, Relative const& unit_vec )const{
                        for(size_t i = 0;; ++i ){
                                auto rel = unit_vec.Times(i);
                                if( ! EmitConditionMoveOrCapture( rel ) )
                                        break;
                        }
                }
                void EmitRook( ChessPieceView* view )const{
                        static std::array<Relative > unit_vec = {
                                Relative{-1, 0 },
                                Relative{ 1, 0 },
                                Relative{ 0,-1 },
                                Relative{ 0, 1 }
                        };
                        for(auto const& vec : unit_vec ){
                                this->EmitForEach( view, vec );
                        }
                }
                void EmitBishop( ChessPieceView* view )const{
                        static std::array<Relative > unit_vec = {
                                Relative{-1,-1 },
                                Relative{-1, 1 },
                                Relative{ 1,-1 },
                                Relative{ 1, 1 }
                        };
                        for(auto const& vec : unit_vec ){
                                this->EmitForEach( view, vec );
                        }
                }
                void EmitQueen( ChessPieceView* view )const{
                        EmitRook(view);
                        EmitBishop(view);
                }
                void EmitKing( ChessPieceView* view ) const{
                        static std::array<Relative, 8> moves = {
                                Relative{ 0, 1},
                                Relative{ 1, 1},
                                Relative{ 1, 0},
                                Relative{ 1,-1},
                                Relative{ 0,-1},
                                Relative{-1,-1},
                                Relative{-1, 0},
                                Relative{-1, 1} 
                        };
                        for(auto const& vec : vmoes ){
                                this->EmitSingle( view, vec );
                        }
                }
        };
        
        
        
        /*
                I'm just doing a toy implementation so, so no imparse moves
         */
        struct PiecePawn : Piece{
                
                virtual char Glyph()const override{ return 'p'; }
                virtual void PossibleMoves(ChessPieceView* view)const override{
                        
                        static MoveRuleBook moveRules;

                        /*
                                Pawn has three moves 4 possibles moves

                                    type         |  condition
                                -----------------+--------------------------------------------
                                move forward     | nothing in front
                                move forward two | nothing in (0,1) and (0,2) and is on 2 rank
                                capture left     | opponent on (1,-1)
                                capture right    | opponent on (1,1)

                                all of these can be coupled with a promotiton when on the 8 rank
                        */


                        if( view->Classify( Relative{0,1} ) == CT_Empty ){

                                switch(view->Rank()){
                                case 7:
                                        view->EmitPromote( Relative{0,1} );
                                        break;
                                case 1:
                                        view->EmitMoveTo( Relative{0,1} );
                                        if( view->Classify( Relative{0,2} ) == CT_Empty ){
                                                view->EmitMoveTo( Relative{0,2} );
                                        }
                                        break;
                                default:
                                        view->EmitMoveTo( Relative{0,1} );
                                        break;
                                }
                        }
                        
                        if( view->Classify( Relative{1,-1} ) == CT_Opponent ){
                                view->EmitCapture( Relative{1,-1} );
                        }
                        if( view->Classify( Relative{1,1} ) == CT_Opponent ){
                                view->EmitCapture( Relative{1,1} );
                        }
                        
                }
        };

        struct PieceKing : Piece{
                virtual char Glyph()const override{ return 'K'; }
                virtual void PossibleMoves(ChessPieceView* view)const override{
                        static MoveRuleBook moveRules;
                        moveRules.EmitKing( view );
                }
        };

        
        struct PieceQueen : Piece{
                virtual char Glyph()const override{ return 'Q'; }
                virtual void PossibleMoves(ChessPieceView* view)const override{
                        static MoveRuleBook moveRules;
                        moveRules.EmitQueen( view );
                }
        };

        struct GameTree{
                ChessInstance* AdoptOrReplace(ChessInstance* instance ){
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
                void LinkAndRecurse(ChessInstance* from, ChessInstance* to){
                        auto next = this->AdoptOrReplace(to);
                        from->Link(next);
                }
                void AddNotify( std::function<void(ChessInstance* instance)> f){
                        on_create_.push_back(std::move(f));
                }
        private:
                std::map<std::string, ChessInstanceHasher*> world_;
                std::vector< std::function<void(Node*)> > on_create_;
        };

        struct TopologyEnumerator{
                explicit TopologyEnumerator(ChessInstance* inst):inst_{inst}{}
                template<class V>
                void ForEachPiece(V&& v){
                        for(cord_t x=0;x!=8;++x){
                                for(cord_t y=0;y!=8;++y){
                                        Absolute abs{x,y};
                                        switch(inst->Classify( inst->Side(), abs )){
                                        case CT_Opponent:
                                        case CT_Friend:
                                                v(inst, abs, inst->Get(abs) );
                                                break;
                                        default:
                                                break;
                                        }
                                }
                        }
                }
                #if 0
                template<class V>
                void FindKing(V&& v, Side side){
                        this->ForEachPiece( [&](ChessInstance* inst, Absolute const& abs, Piece* ptr){
                                if( ptr->Type() == P_King && ptr->Side() == side){
                                        v(inst, abs, ptr);
                                }
                        });
                }
                #endif
        };

        /*
                For example we can't make a move that will put us in check 
        */
        struct CheckDetector{
                bool SideInCheck(ChessInstance* inst, Side side){
                }
        };
        struct TransitionValidator{
                TransitionValidator(ChessInstance* from){
                }
                bool ValidTransition(ChessInstance* to)const{
                }
                bool Terminal(){
                        return terminal_;
                }
        private:
                bool terminal_{false};
        };

        struct BuildManager{
                void Expand( GameTree* gt, ChessInstance* inst ){
                        TopologyEnumerator topo{inst};
                        TransitionValidator validator{inst};

                        topo.ForEachPiece( [&](auto instt, auto const& abs, Piece* ptr){
                                if( ptr->Side() != inst->Side() ){
                                        return;
                                }
                                auto trans = Transform::GetForSide( inst->Side() );
                                std::vector<Move*> moves;
                                ChessPieceView view{ inst, trans, &moves, abs};
                                instr->Get( abs )->PossibleMoves(&view);

                                for(auto move : moves){
                                        ChessInstance* next = inst->Clone();
                                        move->Apply( next );
                                        if( ! validator.ValidTransition(next) ){
                                                continue;
                                        }
                                        gt->LinkAndRecurse( inst, next );
                                }
                        });

                }
        };

        struct GameTreeBuilder{
                std::shared_ptr<GameTree> Build(ChessInstance* root, BuildManager* mgr){
                        auto gt = std::make_shared<GameTree>(root);

                        gt->AddNotify( [this](ChessInstance* inst){
                                stack_.push_back(inst);
                        });
                        stack_.push_back(root);
                        for(;stack.size();){
                                auto inst = stack.back();
                                stack.pop_back();

                                mgr->Expand( this );

                        }
                        
                }
        };

        struct Driver{
                static std::shared_ptr<Driver> Make(){
                        std::shared_ptr<Driver> dvr{new Driver};
                        dvr->instance_ = new ChessInstance{};

                }
        private:
                ChessInstance* instance_;
        };

        /*
                This is asenerious on the chess app.
                Should be simple enough as a test case
         */
        struct EndGameSenerio{
                void Setup( ChessInstance* inst)const{
                        auto pw = Peice::GetPawn(Side_White);
                        auto pb = Peice::GetPawn(Side_Black);

                        inst->Set( Absolute{0,1} ) = pw;
                        inst->Set( Absolute{1,1} ) = pw;
                        inst->Set( Absolute{2,1} ) = pw;
                        
                        inst->Set( Absolute{5,1} ) = pw;
                        inst->Set( Absolute{6,1} ) = pw;
                        inst->Set( Absolute{7,1} ) = pw;
                        
                        inst->Set( Absolute{0,6} ) = pb;
                        inst->Set( Absolute{1,6} ) = pb;
                        
                        inst->Set( Absolute{6,6} ) = pb;
                        inst->Set( Absolute{7,6} ) = pb;

                        inst->Set( Absolute{0,6} ) = Peice::GetKing(Side_White);
                        inst->Set( Absolute{7,6} ) = Peice::GetKing(Side_White);
                }
        };
        

} // end namespace CC

#endif // CHESSINSTANCE_H
