/*
 * song.h
 *
 *  Created on: Sep 15, 2019
 *      Author: blward
 */

#ifndef SONG_H_
#define SONG_H_

#include <inc/notes.h>


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
    short silenceDuration;
    short noteDuration; // Duration of an eighth note in milliseconds (60000/2) / bpm
} Song;


const Note windmillHutNotes[] = {
     { NOTE_REST, 2, REST, BIT7},
     {   NOTE_D5, 2, NONE, BIT0},
     {   NOTE_D5, 2, NONE, BIT0},
     { NOTE_REST, 1, REST, BIT7},
     {   NOTE_E5, 5, NONE, BIT1},
     { NOTE_REST, 2, REST, BIT7},
     {   NOTE_F5, 2, NONE, BIT2},
     {   NOTE_F5, 2, NONE, BIT2},
     { NOTE_REST, 1, REST, BIT7},
     {   NOTE_E5, 5, NONE, BIT1},
     { NOTE_REST, 2, REST, BIT7},
     {   NOTE_D5, 2, NONE, BIT0},
     {   NOTE_D5, 2, NONE, BIT0},
     { NOTE_REST, 1, REST, BIT7},
     {   NOTE_E5, 5, NONE, BIT1},
     { NOTE_REST, 2, REST, BIT7},
     {   NOTE_F5, 2, NONE, BIT2},
     {   NOTE_F5, 2, NONE, BIT2},
     { NOTE_REST, 1, REST, BIT7},
     {   NOTE_E5, 5, NONE, BIT1},
     {   NOTE_D5, 1, NONE, BIT0},
     {   NOTE_F5, 1, NONE, BIT2},
     {   NOTE_D6, 4, NONE, BIT0},
     {   NOTE_D5, 1, NONE, BIT0},
     {   NOTE_F5, 1, NONE, BIT2},
     {   NOTE_D6, 4, NONE, BIT0},
     {   NOTE_E6, 3, NONE, BIT1},
     {   NOTE_F6, 1, STACCATO, BIT2},
     {   NOTE_E6, 1, STACCATO, BIT1},
     {   NOTE_F6, 1, STACCATO, BIT2},
     {   NOTE_E6, 1, NONE, BIT1},
     {   NOTE_C5, 1, STACCATO, BIT3},
     {   NOTE_A5, 4, NONE, BIT2},
     {   NOTE_A5, 1, NONE, BIT2},
     {   NOTE_D5, 1, NONE, BIT0},
     {   NOTE_F5, 1, STACCATO, BIT2},
     {   NOTE_G5, 1, STACCATO, BIT1},
     {   NOTE_A5, 6, NONE, BIT2},
     {   NOTE_A5, 1, NONE, BIT2},
     {   NOTE_D5, 1, NONE, BIT0},
     {   NOTE_F5, 1, STACCATO, BIT2},
     {   NOTE_G5, 1, STACCATO, BIT1},
     {   NOTE_D5, 6, NONE, BIT0},



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
    43,
    20,
    355 // 80 bpm
};



#endif /* SONG_H_ */
