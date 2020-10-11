#include "gmap.hpp"


GMap question1(){
    GMap mgmap = GMap::square();
    mgmap.print_alphas();
    if(mgmap.is_valid()){printf("Map valide\n");}else{printf("Map invalide\n");}
    return mgmap;
}

template<class T>
void print_array(T array, const char * numtype = "%i"){
    printf("[");
    if (!array.empty()){
        if (array.size() > 1){
            for (typename T::const_iterator it = array.begin(); it != array.end()-1 ; ++it){
                printf(numtype,*it);
                printf(",");
            }
        }
        printf(numtype,*(array.end()-1));

    }
    printf("]");
}

void question2(){
    GMap mgmap = question1();
    std::vector<unsigned int> expected = {4,4,1};

    for (unsigned int i = 0 ; i < 3 ; ++i) {
        GMap::idlist_t ielements = mgmap.elements(i);
        printf( "Element de degree %i :",i);
        print_array(ielements);
        printf("\n");
        assert(ielements.size() == expected[i]);
    }
}

GMap3D question3(){
    GMap3D mgmap = GMap3D::square();
    mgmap.print_alphas();
    return mgmap;
}

GMap3D question4a(){
    GMap3D mgmap = GMap3D::cube();
    mgmap.print_alphas();
    return mgmap;
}

GMap3D question4b(){
    GMap3D mgmap = GMap3D::holeshape();
    mgmap.print_alphas();
    return mgmap;
}


void question5(){
    GMap3D mgmap = GMap3D::cube();
    int charac = mgmap.eulercharacteristic();
    printf("Euler characteristic : %i\n", charac);
    printf("Euler Group: %d\n", (int)(-(float)charac/2.0 + 1));
}


void question6a() {
    GMap3D mgmap = question4a();
    display(mgmap);
}
/*
void question6b() {
    GMap3D mgmap = question4b();
    display(mgmap);
}



void question7(){
    GMap3D mgmap = GMap3D::cube();
    mgmap.print_alphas();
    mgmap = mgmap.dual();
    mgmap.print_alphas();
    //display(mgmap);
}
*/
int main(){
    //question1();
    //question2();
    question6a();
    puts("End of execution");
    return 0;
}
