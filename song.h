/*
 * song.h
 *
 *  Created on: Sep 15, 2019
 *      Author: blward
 */

#ifndef SONG_H_
#define SONG_H_

#include "notes.h"


typedef enum NoteModifier {
    NONE = 0,
    REST = 1,
    STACCATO = 2
} NoteModifier;

typedef struct Note
{
    int frequency; // Frequency in Hz
    int eighths; // Duration in sixteenth notes
    short modifier;
    int button;
} Note;

typedef struct Song
{
    const Note* notes;
    short noteCount;
    short msBetweenNotes;
} Song;


const Note windmillHutNotes[] = {
//     { NOTE_REST, 2, REST},
//     {   NOTE_D5, 2, NONE},
//     {   NOTE_D5, 2, NONE},
//     { NOTE_REST, 1, REST},
//     {   NOTE_E5, 5, NONE},
//     { NOTE_REST, 2, REST},
//     {   NOTE_F5, 2, NONE},
//     {   NOTE_F5, 2, NONE},
//     { NOTE_REST, 1, REST},
//     {   NOTE_E5, 5, NONE},
//     { NOTE_REST, 2, REST},
//     {   NOTE_D5, 2, NONE},
//     {   NOTE_D5, 2, NONE},
//     { NOTE_REST, 1, REST},
//     {   NOTE_E5, 5, NONE},
//     { NOTE_REST, 2, REST},
//     {   NOTE_F5, 2, NONE},
//     {   NOTE_F5, 2, NONE},
//     { NOTE_REST, 1, REST},
//     {   NOTE_E5, 5, NONE},
     {   NOTE_D5, 1, NONE, 1},
     {   NOTE_F5, 1, NONE, 1},
     {   NOTE_D6, 4, NONE, 1},
     {   NOTE_D5, 1, NONE, 1},
     {   NOTE_F5, 1, NONE, 1},
     {   NOTE_D6, 4, NONE, 1},
     {   NOTE_E6, 3, NONE, 1},
     {   NOTE_F6, 1, STACCATO, 1},
     {   NOTE_E6, 1, STACCATO, 1},
     {   NOTE_F6, 1, STACCATO, 1},
     {   NOTE_E6, 1, NONE, 1},
     {   NOTE_C5, 1, STACCATO, 1},
     {   NOTE_A5, 4, NONE, 1},
     {   NOTE_A5, 1, NONE, 1},
     {   NOTE_D5, 1, NONE, 1},
     {   NOTE_F5, 1, STACCATO, 1},
     {   NOTE_G5, 1, STACCATO, 1},
     {   NOTE_A5, 6, NONE, 1},
     {   NOTE_A5, 1, NONE, 1},
     {   NOTE_D5, 1, NONE, 1},
     {   NOTE_F5, 1, STACCATO, 1},
     {   NOTE_G5, 1, STACCATO, 1},
     {   NOTE_D5, 6, NONE, 1},



//        { NOTE_REST, 2, REST},
//        {   NOTE_E6, 2, NONE},
//        {   NOTE_F6, 2, NONE},
//        {   NOTE_E6, 100, NONE},
//        {   NOTE_F6, 100, NONE},
//        {   NOTE_E6, 100, NONE},
//        {   NOTE_C6, 100, NONE},
//        {   NOTE_A5, 100, NONE},
//        {   NOTE_D5, 200, NONE},
//        {   NOTE_F5, 100, NONE},
//        {   NOTE_G5, 100, NONE},
//        {   NOTE_A5, 100, NONE},
//        {   NOTE_A5, 200, NONE},
//        {   NOTE_D5, 200, NONE},
//        {   NOTE_F5, 100, NONE},
//        {   NOTE_G5, 100, NONE},
//        {   NOTE_E5, 100, NONE},
 };

const Song windmillHut =
{
    windmillHutNotes,
    23,
    5
};



#endif /* SONG_H_ */
