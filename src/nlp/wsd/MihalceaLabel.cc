/*
 * MihalceaLabel.cc
 *
 * Implements the word-instance labelling portion of the Rada Mihalcea
 * word-sense disambiguation algorithm.
 *
 * Copyright (c) 2008 Linas Vepstas <linas@linas.org>
 */
#include <stdio.h>

#include "FollowLink.h"
#include "ForeachWord.h"
#include "MihalceaLabel.h"
#include "Node.h"
#include "SimpleTruthValue.h"

using namespace opencog;

MihalceaLabel::MihalceaLabel(void)
{
	atom_space = NULL;
}

MihalceaLabel::~MihalceaLabel()
{
	atom_space = NULL;
}

/**
 * Anotate every word in every parse of the sentence with every possible
 * word sense for that word. The argument handle is presumed to identify
 * a SentenceNode.
 */
void MihalceaLabel::annotate_sentence(Handle h)
{
	foreach_parse(h, &MihalceaLabel::annotate_parse, this);
}

/**
 * Anotate every word in the given parse with every possible word sense
 * for that word. The argument handle is presumed to identify a specific
 * parse.
 */
bool MihalceaLabel::annotate_parse(Handle h)
{
printf("found parse %x\n", (unsigned int) h);
	foreach_word_instance(h, &MihalceaLabel::annotate_word, this);
	return false;
}

/**
 * Anotate the given word with every possible word sense, given its 
 * part-of-speech. The argument handle is assumed to point at a specific 
 * word-instance in some parse.
 */
bool MihalceaLabel::annotate_word(Handle h)
{
	word_instance = TLB::getAtom(h);

Node *n = dynamic_cast<Node *>(word_instance);
printf("found word-inst %s\n",  n->toString().c_str());

	// Find the part-of-speech for this word instance.
	std::string word_inst_pos = get_part_of_speech(h);

	// Reject some unwanted parts-of-speech.
	if (0 == word_inst_pos.compare("WORD")) return false;
	if (0 == word_inst_pos.compare("det")) return false;
	if (0 == word_inst_pos.compare("particle")) return false;
	if (0 == word_inst_pos.compare("prep")) return false;
	if (0 == word_inst_pos.compare("punctuation")) return false;

printf("found inst-pos %s\n",  word_inst_pos.c_str());

	Handle dict_word_h = get_dict_word_of_word_instance(h);

n = dynamic_cast<Node *>(TLB::getAtom(dict_word_h));
printf("found word-dict %s\n",  n->toString().c_str());
 
	// loop over all word senses with this part-of-speech.
	foreach_dict_word_sense_pos(dict_word_h, word_inst_pos,
	                        &MihalceaLabel::annotate_word_sense, this);
	return false;
}

/**
 * Create a link coupling a specific word-instance to a possible 
 * word-sense.  The link to be created will resemble the following:
 *
 *   <InheritanceLink strength=0.9 confidence=0.1>
 *      <ConceptNode name="bark_144" />
 *      <WordSenseNode name="bark_sense_23" />
 *   </InheritanceLink>
 */
bool MihalceaLabel::annotate_word_sense(Handle h)
{
	Atom *word_sense = TLB::getAtom(h);

Node *n = dynamic_cast<Node *>(word_sense);
printf("found word-sense %s\n",  n->toString().c_str());

	// Create a link connecting this word-instance to this word-sense.
	std::vector<Handle> out;
	out.push_back(TLB::getHandle(word_instance));
	out.push_back(TLB::getHandle(word_sense));

	// Give it a false truth value, very low confidence.
	SimpleTruthValue stv(0.0, 1.0);
	stv.setConfidence(0.01);
	atom_space->addLink(INHERITANCE_LINK, out, stv);

	return false;
}

/* ============================== END OF FILE ====================== */
