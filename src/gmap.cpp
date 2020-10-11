#include "gmap.hpp"
/*------------------------------------------------------------------------*/

// Return the application of the alpha_deg on dart
GMap::id_t GMap::alpha(degree_t degree, id_t dart) const
{
	assert(degree < 3);
	assert(dart < maxid);
	return alphas.at(dart)[degree];
}

// Return the application of a composition of alphas on dart
GMap::id_t GMap::alpha(degreelist_t degrees, id_t dart) const
{
	for(degree_t degree : degrees){
		assert(degree < 3);
		assert(dart < maxid);
		dart = alphas.at(dart)[degree];
	}
	return dart;
}


//  Test if dart is free for alpha_degree (if it is a fixed point)
bool GMap::is_free(degree_t degree, id_t dart) const
{
	assert(degree < 3);
	assert(dart < maxid);
	return alpha(degree, dart) == dart;
}

/*
    Test the validity of the structure.
    Check that alpha_0 and alpha_1 are involutions with no fixed points.
*/
bool GMap::is_valid() const
{
	// Check alpha_0 et alpha_1
	for(idalphamap_t::const_iterator it = alphas.begin() ; it != alphas.end() ; it++){
		const id_t& dart = it->first;
		for(degree_t degree = 0 ; degree < 2 ; degree++){
			const id_t& alpha_deg_of_dart = it->second[degree]; // Brin atteint par alpha_degree
			if(dart == alpha_deg_of_dart) return false; // Check pas de point fixe
			if(dart != alpha(degree, alpha_deg_of_dart)) return false; // Check est une involution
		}
	}

	// Check alpha_2
	degreelist_t alphas = {0, 2, 0, 2};
	for(id_t dart : darts()){
		if(alpha(alphas, dart) != dart)
			return false;
	}

	return true;
}

/*
    Create a new dart and return its id.
    Set its alpha_i to itself (fixed points)
*/
GMap::id_t GMap::add_dart()
{
	id_t dart = maxid;
	++maxid;
	alphas[dart] = alpha_container_t(dart, dart, dart);
	return dart;
}

// Link the two darts with a relation alpha_degree if they are both free.
bool GMap::link_darts(degree_t degree, id_t dart1, id_t dart2)
{
	if(this->is_free(degree, dart1) && this->is_free(degree, dart2)){
		this->alphas.at(dart1)[degree] = dart2;
		this->alphas.at(dart2)[degree] = dart1;

		return true;
	}
	else{
		return false;
	}
}

/*
    Return the orbit of dart using a list of alpha relation.
    Example of use : gmap.orbit(0,[0,1]).
*/
GMap::idlist_t GMap::orbit(degreelist_t list_of_alpha_value, id_t dart) const
{
	idlist_t result;
	idset_t marked;
	idlist_t toprocess = {dart};

	while(!toprocess.empty()){
		id_t d = toprocess.front();
		toprocess.erase(toprocess.begin());
		if(marked.count(d) == 0){
			result.push_back(d);
			marked.insert(d);
			for(degree_t degree : list_of_alpha_value){
				toprocess.push_back(this->alpha(degree, d));
			}
		}
	}
	return result;
}

/*
    Return the ordered orbit of dart using a list of alpha relations by applying
    repeatingly the alpha relations of the list to dart.
    Example of use. gmap.orderedorbit(0,[0,1]).
    Warning: No fixed point for the given alpha should be contained.
*/

GMap::idlist_t GMap::orderedorbit(degreelist_t list_of_alpha_value, id_t dart)
{
	idlist_t result;
    id_t current_dart = dart;
    unsigned char current_alpha_index = 0;
    size_t n_alpha = list_of_alpha_value.size();

	do{
		result.push_back(current_dart);
		current_dart = this->alpha(list_of_alpha_value[current_alpha_index], current_dart);
		current_alpha_index = (current_alpha_index + 1) % n_alpha;
	}while(current_dart != dart);

	return result;
}


/*
    Sew two elements of degree 'degree' that start at dart1 and dart2.
    Determine first the orbits of dart to sew and check if they are compatible.
    Sew pairs of corresponding darts
    # and if they have different embedding  positions, merge them.
*/
bool GMap::sew_dart(degree_t degree, id_t dart1, id_t dart2)
{
	if(degree == 1){
		this->link_darts(degree, dart1, dart2);
	}else if((degree == 0) || (degree == 2)){
		degree_t oppose = (degree == 0) ? (2) : (0);
		idlist_t orbit_1 = this->orbit({oppose}, dart1);
		idlist_t orbit_2 = this->orbit({oppose}, dart2);

		if(orbit_1.size() != orbit_2.size()){
			return false;
		}
		else{
			for(u_int i = 0 ; i < orbit_1.size() ; i++){
				this->link_darts(degree, orbit_1[i], orbit_2[i]);
			}
			return true;
		}
	}else{
		return false;
	}
}

// Compute the Euler-Poincare characteristic of the subdivision
int GMap::eulercharacteristic()
{
	u_int sommets = this->elements(0).size();
	u_int aretes = this->elements(1).size();
	u_int faces = this->elements(2).size();

	printf("Sommets: %d, Arêtes: %d, Faces: %d\n", sommets, aretes, faces);
	return sommets - aretes + faces;
}


/*------------------------------------------------------------------------*/

/*
    Check if a dart of the orbit representing the vertex has already been
    associated with a value in propertydict. If yes, return this dart, else
    return the dart passed as argument.
*/
/*
template<class T>
GMap::id_t EmbeddedGMap<T>::get_embedding_dart(id_t dart)
{
}
*/


/*------------------------------------------------------------------------*/

GMap3D GMap3D::dual()
{
}


/*------------------------------------------------------------------------*/
