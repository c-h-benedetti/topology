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
	for(idalphamap_t::const_iterator it = alpha.begin() ; it != alpha.end() ; std::next(it, 1)){
		const id_t& dart = it->first();
		for(degree_t degree = 0 ; degree < 2 ; degree++){
			const id_t& alpha_deg_of_dart = it->second[degree];
			if(dart == alpha_deg_of_dart) return false;
			if(dart != alpha(degree, alpha_deg_of_dart)) return false;
		}
	}

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
	
}

/* 
    Return the orbit of dart using a list of alpha relation.
    Example of use : gmap.orbit(0,[0,1]).
*/
GMap::idlist_t GMap::orbit(degreelist_t alphas, id_t dart)
{

}

/*
    Return the ordered orbit of dart using a list of alpha relations by applying
    repeatingly the alpha relations of the list to dart.
    Example of use. gmap.orderedorbit(0,[0,1]).
    Warning: No fixed point for the given alpha should be contained.
*/
GMap::idlist_t GMap::orderedorbit(degreelist_t list_of_alpha_value, id_t dart)
{

}


/*
    Sew two elements of degree 'degree' that start at dart1 and dart2.
    Determine first the orbits of dart to sew and heck if they are compatible.
    Sew pairs of corresponding darts
    # and if they have different embedding  positions, merge them. 
*/
bool GMap::sew_dart(degree_t degree, id_t dart1, id_t dart2)
{
}

// Compute the Euler-Poincare characteristic of the subdivision
int GMap::eulercharacteristic()
{
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
