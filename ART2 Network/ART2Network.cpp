/*
 * ART2Network.cpp
 *
 *  Created on: Apr 29, 2013
 *      Author: Tony
 */

#include "ART2Network.h"
#include "nnfunctions.h"

using namespace art2nn;

ART2Network::ART2Network(dimension input_dimension, param a, param b, param c, param d, param e, param theta, param rho):
	input_dimension(input_dimension),
	a(a), b(b), c(c), d(d), e(e), theta(theta), rho(rho),
	f(LINEAR_SIGNAL_FUNCTION), g(HEAVISIDE_SIGNAL_FUNCTION),
	F1(*this), F2(*this) {
}

ART2Network::~ART2Network() {
}

ART2Network::


/* Layer1 */

ART2Network::Layer1::Layer1(const ART2Network &parent):
	parent(parent),
	p(parent.input_dimension),
	q(parent.input_dimension),
	u(parent.input_dimension),
	v(parent.input_dimension),
	w(parent.input_dimension),
	x(parent.input_dimension) {
}

signal_vector ART2Network::Layer1::operator()(input_vector I) {
	param a = parent.a;
	param b = parent.b;
	param e = parent.e;
	param theta = parent.theta;

	signal (*f)(param, signal) = parent.f;
	signal (*g)(param, signal) = parent.g;

	const weight_matrix &W = parent.W;

	signal_vector y = parent.F2.output();

	signal_vector temp_p(u + W * vectorApply(y, g, theta));
	signal_vector temp_q(p * (e + p.norm()));
	signal_vector temp_u(v * (e + v.norm()));
	signal_vector temp_v(vectorApply(x, f, theta) + b * vectorApply(q, f, theta));
	signal_vector temp_w(I + a * u);
	signal_vector temp_x(w * (e + w.norm()));

	p = temp_p;
	q = temp_q;
	u = temp_u;
	v = temp_v;
	w = temp_w;
	x = temp_x;

	return p;
}

void ART2Network::Layer1::zeroInput() {
	dimension m = parent.input_dimension;
	for (index i = 0; i < m; ++i)
		p[i] = q[i] = u[i] = v[i] = w[i] = x[i] = 0.0;
}


/* Layer 2 */

ART2Network::Layer2::Layer2(const ART2Network &parent):
	Maxnet(), parent(parent) {
}

void ART2Network::Layer2::reset() {
	param epsilon = 1.0 / node_count;
	param theta = 1.0;

	for (index i = 0; i < node_count; ++i) {
		for (index j = 0; j < node_count; ++j) {
			if (i != j)
				W(i, j, -epsilon);
			else
				W(i, j, theta);
		}
	}
}

void ART2Network::Layer2::suppress(index j) {
	for (index i = 0; i < node_count; ++i)
		W(i, j, 0.0);
}

unsigned int ART2Network::Layer2::unsuppressedNodeCount() {
	unsigned int count = 0;
	for (index i = 0; i < node_count; ++i) {
		if (W(i, 0) == 0.0)
			++count;
	}
	return count;
}

void ART2Network::Layer2::addNode() {
	++node_count;
	signals = signals.project(node_count);

	weight_matrix newW(node_count, node_count);
	W = newW;
	reset();
}

/* Vigil */

ART2Network::Vigil::Vigil(const ART2Network& parent):
	parent(parent), r(parent.input_dimension) {
}

bool ART2Network::Vigil::operator()() {
	param c = parent.c;
	param e = parent.e;
	param rho = parent.rho;

	const signal_vector &u = parent.F1.u;
	const signal_vector &p = parent.F1.p;

	r = (u + c*p) / (e + u.norm() + c * p.norm());

	return rho / (e + r.norm()) > 1;
}
